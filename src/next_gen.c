#include <stdio.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include "my_sha.h"

void next_stem(char *input) {
  int index = 3;

  int done = 0;
  while(!done) {
    if(input[index] == 126) {
      input[index] = '!';
      index--;
    } else {
      input[index]++;
      done = 1;
    }
  }
}

uint32_t atou(char *s) {
  return (uint32_t) strtoul(s, (char **) NULL, 10);
}

void vectorize_prefixes(struct vector_ctx *vc, SHA_CTX *prefix_ctxs) {
    vc->h0.u32[0] = prefix_ctxs[0].h0;
    vc->h0.u32[1] = prefix_ctxs[1].h0;
    vc->h0.u32[2] = prefix_ctxs[2].h0;
    vc->h0.u32[3] = prefix_ctxs[3].h0;

    vc->h1.u32[0] = prefix_ctxs[0].h1;
    vc->h1.u32[1] = prefix_ctxs[1].h1;
    vc->h1.u32[2] = prefix_ctxs[2].h1;
    vc->h1.u32[3] = prefix_ctxs[3].h1;

    vc->h2.u32[0] = prefix_ctxs[0].h2;
    vc->h2.u32[1] = prefix_ctxs[1].h2;
    vc->h2.u32[2] = prefix_ctxs[2].h2;
    vc->h2.u32[3] = prefix_ctxs[3].h2;

    vc->h3.u32[0] = prefix_ctxs[0].h3;
    vc->h3.u32[1] = prefix_ctxs[1].h3;
    vc->h3.u32[2] = prefix_ctxs[2].h3;
    vc->h3.u32[3] = prefix_ctxs[3].h3;

    vc->h4.u32[0] = prefix_ctxs[0].h4;
    vc->h4.u32[1] = prefix_ctxs[1].h4;
    vc->h4.u32[2] = prefix_ctxs[2].h4;
    vc->h4.u32[3] = prefix_ctxs[3].h4;
}


// ./next_gen "# of permutations" stem0 stem1 stem2 ...
int main(int argc, unsigned char **argv) {
  char phrase[59] = "I would much rather hear more about your whittling project";
  unsigned int permutations = atou((char *)argv[1]);

  sseK00_19 = _mm_set1_epi32(0x5a827999);
  sseK20_39 = _mm_set1_epi32(0x6ed9eba1);
  sseK40_59 = _mm_set1_epi32(0x8f1bbcdc);
  sseK60_79 = _mm_set1_epi32(0xca62c1d6);

  best_stem  = malloc(sizeof(char) * 5);

  shortest_d = 180;

  // Get finished context for phrase
  SHA_CTX *phrase_ctx = malloc(sizeof(SHA_CTX));
  sha1_full(phrase_ctx, phrase);

  // Load prefixes
  unsigned char **prefixes = malloc(sizeof(char *) * PREFIX_COUNT);
  int p = 0;
  for(p = 0;p < PREFIX_COUNT;p++) 
    prefixes[p] = argv[2 + p];

  // Get chaining contexts for suffixes
  SHA_CTX *prefix_ctxs = malloc(sizeof(SHA_CTX) * PREFIX_COUNT);
  for(p = 0;p < PREFIX_COUNT;p++) 
    sha1_partial(&prefix_ctxs[p], prefixes[p]);

  struct vector_ctx *vc = malloc(sizeof(struct vector_ctx) * PREFIX_COUNT/4);
  struct vector_ctx *vc_ptr = vc;

  SHA_CTX *prefix_ptr = prefix_ctxs;
  for(p = 0;p < PREFIX_COUNT;p+=4) {
    vectorize_prefixes(vc_ptr, prefix_ptr);

    prefix_ptr += 4;
    vc_ptr += 1;
  }


  // Allocate memory for expanded message template
  uint32_t *w = malloc(sizeof(uint32_t) * 80);
  
  int w_i = 0;

  // We only hash suffixes that are 5 bytes long
  
  // w[0] prefix_stem
  // w[1] current final char + some other stuff and zeros
  // w[2]-w[14]
  // Expanded message blocks 2-14 are always 0x0000000...
  for(w_i = 2;w_i < 15;w_i++)
    w[w_i] = 0;

  // w[15] is the size of the message
  w[15] = 552;
  // w[16] - stem constant - W13 ^ W8 ^ W2 ^ W0  => W0 <<< 1
  // w[17] - changing lots
  // w[18] - constant
  w[18] = ROTATE(w[15], 1);
  // w[19] - stem constant
  // w[20] - changing lots 


  uint32_t *stem_w = malloc(sizeof(uint32_t) * 80);
  uint32_t *stem_x = malloc(sizeof(uint32_t) * 80);

  init_lut();

  struct vector_ctx *my_vc = malloc(sizeof(struct vector_ctx) * PREFIX_COUNT/2);
  int i = 0;
  char suffix_stem[5] = "!!!!";
  for(i = 0;i < permutations;i++) {
    memcpy(stem_w, w, 80 * sizeof(uint32_t));
    memcpy(my_vc, vc, sizeof(struct vector_ctx) * PREFIX_COUNT/2);

    int dist = shortest_distance(phrase_ctx, my_vc, suffix_stem, stem_w, stem_x);

    next_stem(suffix_stem);
  }

  free(vc);
  free(my_vc);
  free(w);
  free(stem_w);
  free(stem_x);

  // Print shortest_distance and the 5 char ending.
  printf("%d,%s,%d\n", shortest_d, best_stem, best_last+33);
}
