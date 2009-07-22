#include <stdint.h>
#include <asm/byteorder.h>
#include <xmmintrin.h>
#include <sys/time.h>

#define PREFIX_COUNT 32 

#define ROTATE(a,n)  ({ register unsigned int ret; \
		asm (     \
		"roll %1,%0"    \
		: "=r"(ret)   \
		: "I"(n), "0"(a)  \
		: "cc");    \
		ret;       \
		})


#define SW32(c) ___arch__swab32(c)

__m128i sseK00_19;
__m128i sseK20_39;
__m128i sseK40_59;
__m128i sseK60_79;

typedef union {
   uint32_t u32[4];
   __m128i u128;
} v4si __attribute__((aligned(16)));

struct vector_ctx {
  v4si h0;
  v4si h1;
  v4si h2;
  v4si h3;
  v4si h4;
};

int shortest_d;
int best_last;
char *best_stem;

#define K_00_19 0x5a827999UL
#define K_20_39 0x6ed9eba1UL
#define K_40_59 0x8f1bbcdcUL
#define K_60_79 0xca62c1d6UL
