#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "my_sha.h"


#define BX_(x) ((x) - (((x)>>1)&0x77777777) \
                 - (((x)>>2)&0x33333333) \
                 - (((x)>>3)&0x11111111))

#define BITCOUNT(x) (((BX_(x)+(BX_(x)>>4)) & 0x0F0F0F0F) % 255)


// OPENSSL based convenience
void sha1_full(SHA_CTX *ctx, unsigned char *phrase) {
	unsigned char *useless = malloc(sizeof(char) * 20);
	SHA1_Init(ctx);
  SHA1_Update(ctx, phrase, sizeof(char) * 58);
  SHA1_Final(useless, ctx);
}

// prefix - 64 bytes
void sha1_partial(SHA_CTX *ctx, unsigned char *prefix) {
	SHA1_Init(ctx);
  SHA1_Update(ctx, prefix, sizeof(char) * 64);
}

void print_expanded_message(uint32_t *msg) {
	int i = 0;
	for(i; i < 80;i++) {
		printf("w%d 0x%08x\n", i, msg[i]);
	}
}

unsigned char lut[65536];
static inline int popcount_lut16(uint32_t buf)
{
  int cnt=0;
  //do {
    cnt += lut[buf&65535];
    cnt += lut[buf>>16];
    buf++;
  //} while(--n);
  return cnt;
}

void init_lut(void)
{
    int i;
    for(i=0;i<65536;i++) {
      lut[i] = BITCOUNT(i);
   }
}

int hamming_distance_vc(SHA_CTX *phrase_ctx, struct vector_ctx *vc) {
  __m128i p = _mm_set1_epi32(phrase_ctx->h0);
  vc->h0.u128 = _mm_xor_si128(vc->h0.u128, p);

  p = _mm_set1_epi32(phrase_ctx->h1);
  vc->h1.u128 = _mm_xor_si128(vc->h1.u128, p);

  p = _mm_set1_epi32(phrase_ctx->h2);
  vc->h2.u128 = _mm_xor_si128(vc->h2.u128, p);

  p = _mm_set1_epi32(phrase_ctx->h3);
  vc->h3.u128 = _mm_xor_si128(vc->h3.u128, p);

  p = _mm_set1_epi32(phrase_ctx->h4);
  vc->h4.u128 = _mm_xor_si128(vc->h4.u128, p);

  int found_shortest = 0;

  int i = 0;
  for(i; i < 4;i++) {
    int dist = 0;
    dist += popcount_lut16(vc->h0.u32[i]);
    dist += popcount_lut16(vc->h1.u32[i]);
    dist += popcount_lut16(vc->h2.u32[i]);
    dist += popcount_lut16(vc->h3.u32[i]);
    dist += popcount_lut16(vc->h4.u32[i]);
    //printf("%d\n", dist);
    if(dist < shortest_d) {
      shortest_d = dist;
      found_shortest = 1;
     // printf("found: %d\n", shortest_d);
    }
  }

  return found_shortest;
}

inline int shortest_distance(SHA_CTX *phrase_ctx, struct vector_ctx *vc, char *suffix_stem, uint32_t *stem_w, uint32_t *stem_x) {
	int shortest = 180;

  // Build expanded message for suffix stem
	stem_w[0] = (suffix_stem[0] << 24) + (suffix_stem[1] << 16) + (suffix_stem[2] << 8) + suffix_stem[3];
	// w[16] => w[0] <<< 1
	stem_w[16] = ROTATE(stem_w[0], 1);
	// w[17] below
	// w[18] => w[15] <<< 1
	stem_w[18] = ROTATE(stem_w[15], 1);
	// w[19] => w[16] <<< 1
	stem_w[19] = ROTATE(stem_w[16], 1);
  // w[21] => w[18] <<< 1
	stem_w[21] = ROTATE(stem_w[18], 1);
  // w[22] => w[19] <<< 1
	stem_w[22] = ROTATE(stem_w[19], 1);
  // w[24] => w16 ^ w21
	stem_w[24] = ROTATE(stem_w[16] ^ stem_w[21], 1);

  stem_w[0] += K_00_19;

  struct vector_ctx *my_vc = malloc(sizeof(struct vector_ctx));
  v4si *stemm = malloc(sizeof(v4si) * 80);
	int f = 0;
	for(f; f < 94;f++) {
		char last = f + '!';
//    gettimeofday(&expand_start,0);
		stem_w[1] = (last << 24) + (1 << 23);
		// TODO LU?
		stem_w[17] = ROTATE(stem_w[1], 1);
		// TODO LU?
		stem_w[20] = ROTATE(stem_w[17], 1);
    // w[23] => w[20] ^ w[15]
    stem_w[23] = ROTATE(stem_w[20] ^ stem_w[15], 1);

		int g = 25;
		for(g; g < 80;g++)
			stem_w[g] = ROTATE((stem_w[g-3] ^ stem_w[g-8] ^ stem_w[g-14] ^ stem_w[g-16]),1);

    stemm[0].u128 = _mm_set1_epi32(stem_w[0]); 

		// Precompute the K addition for each round 
    for(g = 1; g < 20;g++) {
      stemm[g].u128 = _mm_set1_epi32(stem_w[g]); 
      stemm[g].u128 = _mm_add_epi32(stemm[g].u128, sseK00_19);
    }
    for(g = 20; g < 40;g++) {
      stemm[g].u128 = _mm_set1_epi32(stem_w[g]); 
      stemm[g].u128 = _mm_add_epi32(stemm[g].u128, sseK20_39);
    }
    for(g = 40; g < 60;g++) {
      stemm[g].u128 = _mm_set1_epi32(stem_w[g]); 
      stemm[g].u128 = _mm_add_epi32(stemm[g].u128, sseK40_59);
    }
    for(g = 60; g < 80;g++) {
      stemm[g].u128 = _mm_set1_epi32(stem_w[g]); 
      stemm[g].u128 = _mm_add_epi32(stemm[g].u128, sseK60_79);
    }

    struct vector_ctx *vc_ptr = vc;
    int p = 0;
    for(p; p < PREFIX_COUNT;p+=4) {
      memcpy(my_vc, vc_ptr, sizeof(struct vector_ctx));
      update_context_sse(my_vc, stemm);
    
      int found_shortest = hamming_distance_vc(phrase_ctx, my_vc);

      if(found_shortest) {
    //    printf("f: %d\n", f);
        sprintf(best_stem, "%s", suffix_stem);
        best_last = f;
      }

      vc_ptr+=1;
    }
    
	}

  free(stemm);
  free(my_vc);
}
