#include "my_sha.h"
#include <stdint.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

#define X(a) w[a]

	//printf("%d a: %x b: %x c: %x d: %x e: %x t: %x w: %x\n", i, a, b, c,d,e,f, X(i)); 
#define BODY_00_15(i,a,b,c,d,e,f) \
	(f)=X(i)+(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d)); \
	(b)=ROTATE((b),30);

#define BODY_16_19(i,a,b,c,d,e,f) \
	(f)=X(i)+(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d)); \
	(b)=ROTATE((b),30);

#define BODY_20_31(i,a,b,c,d,e,f) \
	(f)=X(i)+(e)+K_20_39+ROTATE((a),5)+F_20_39((b),(c),(d)); \
	(b)=ROTATE((b),30);

#define BODY_32_39(i,a,b,c,d,e,f) \
	(f)=X(i)+(e)+K_20_39+ROTATE((a),5)+F_20_39((b),(c),(d)); \
	(b)=ROTATE((b),30);

#define BODY_40_59(i,a,b,c,d,e,f) \
	(f)=X(i)+(e)+K_40_59+ROTATE((a),5)+F_40_59((b),(c),(d)); \
	(b)=ROTATE((b),30);

#define BODY_60_79(i,a,b,c,d,e,f) \
	(f)=X(i)+(e)+K_60_79+ROTATE((a),5)+F_60_79((b),(c),(d)); \
	(b)=ROTATE((b),30);

inline __m128i _mm_rotl_epi32(__m128i x, int bits)
{
	return _mm_or_si128(_mm_slli_epi32(x, bits), _mm_srli_epi32(x, 32 - bits));
}

inline void update_context_sse(struct vector_ctx *vc, v4si *stemm) {

  v4si A = vc->h0;
  v4si B = vc->h1;
  v4si C = vc->h2;
  v4si D = vc->h3;
  v4si E = vc->h4;
  v4si T;

	/*
(0  ≤ i ≤ 19): f = d xor (b and (c xor d))                (alternative 1) _mm_xor_si128((d), _mm_and_si128((b), _mm_xor_si128((c),(d))));
(0  ≤ i ≤ 19): f = (b and c) xor ((not b) and d)          (alternative 2) _mm_xor_si128(_mm_and_si128((b),(c)), _mm_pand_si128((b), (d)));
(0  ≤ i ≤ 19): f = (b and c) + ((not b) and d)            (alternative 3) _mm_add_si128(_mm_and_si128((b),(c)), _mm_pand_si128((b), (d)));
(0  ≤ i ≤ 19): f = vec_sel(d, c, b)                       (alternative 4)
*/

  /*
 (40 ≤ i ≤ 59): f = (b and c) or (d and (b or c))          (alternative 1)
(40 ≤ i ≤ 59): f = (b and c) or (d and (b xor c))         (alternative 2)
(40 ≤ i ≤ 59): f = (b and c) + (d and (b xor c))          (alternative 3)
(40 ≤ i ≤ 59): f = (b and c) xor (b and d) xor (c and d)  (alternative 4)
*/
	
	// one of these is faster than the others (by a little)
//#define F_00_19_epi32(b,c,d)   _mm_add_epi32(_mm_and_si128((b),(c)), _mm_andnot_si128((b), (d)));
#define F_00_19_epi32(b,c,d)   _mm_xor_si128(_mm_and_si128((b),(c)), _mm_andnot_si128((b), (d)))
//#define F_00_19_epi32(b,c,d)  _mm_xor_si128(_mm_and_si128(_mm_xor_si128((c),(d)), (b)), (d));

#define F_20_39_epi32(b,c,d)  _mm_xor_si128(_mm_xor_si128(b,c),d)

#define F_40_59_epi32(b,c,d)	_mm_or_si128(_mm_and_si128(b,c), (_mm_and_si128(_mm_xor_si128(b,c), d)))

#define	F_60_79_epi32(b,c,d)	F_20_39_epi32(b,c,d)

  //printf("%d a: %x b: %x c: %x d: %x e: %x\n",i , a.u32[0], b.u32[0], c.u32[0], d.u32[0], e.u32[0]);
  
#define BODY_00_15_epi32(i,a,b,c,d,e,f) \
  f.u128 = _mm_add_epi32(stemm[i].u128, e.u128); \
  f.u128 = _mm_add_epi32(f.u128, _mm_rotl_epi32(a.u128, 5)); \
  f.u128 = _mm_add_epi32(f.u128, F_00_19_epi32(b.u128,c.u128,d.u128)); \
  b.u128 = _mm_rotl_epi32(b.u128, 30);
  
#define BODY_00_15_epi32_no_f(i,a,b,c,d,e,f) \
  f.u128 = _mm_add_epi32(stemm[i].u128, e.u128); \
  f.u128 = _mm_add_epi32(f.u128, _mm_rotl_epi32(a.u128, 5)); \
  f.u128 = _mm_add_epi32(f.u128, F_00_19_epi32(b.u128,c.u128,d.u128)); \
  b.u128 = _mm_rotl_epi32(b.u128, 30);
  
#define BODY_00_15_epi32_no_w(i,a,b,c,d,e,f) \
  f.u128 = _mm_add_epi32(stemm[i].u128, e.u128); \
  f.u128 = _mm_add_epi32(f.u128, F_00_19_epi32(b.u128,c.u128,d.u128)); \
  f.u128 = _mm_add_epi32(f.u128, _mm_rotl_epi32(a.u128, 5)); \
  b.u128 = _mm_rotl_epi32(b.u128, 30); \

	BODY_00_15_epi32_no_f( 0,A,B,C,D,E,T);
	BODY_00_15_epi32( 1,T,A,B,C,D,E);
	BODY_00_15_epi32_no_w( 2,E,T,A,B,C,D);
	BODY_00_15_epi32_no_w( 3,D,E,T,A,B,C);
	BODY_00_15_epi32_no_w( 4,C,D,E,T,A,B);
	BODY_00_15_epi32_no_w( 5,B,C,D,E,T,A);
	BODY_00_15_epi32_no_w( 6,A,B,C,D,E,T);
	BODY_00_15_epi32_no_w( 7,T,A,B,C,D,E);
	BODY_00_15_epi32_no_w( 8,E,T,A,B,C,D);	
	BODY_00_15_epi32_no_w( 9,D,E,T,A,B,C);	
	BODY_00_15_epi32_no_w(10,C,D,E,T,A,B);	
	BODY_00_15_epi32_no_w(11,B,C,D,E,T,A);
	BODY_00_15_epi32_no_w(12,A,B,C,D,E,T);
	BODY_00_15_epi32_no_w(13,T,A,B,C,D,E);
	BODY_00_15_epi32_no_w(14,E,T,A,B,C,D);
	BODY_00_15_epi32(15,D,E,T,A,B,C);
	BODY_00_15_epi32(16,C,D,E,T,A,B); //16...
	BODY_00_15_epi32(17,B,C,D,E,T,A);
	BODY_00_15_epi32(18,A,B,C,D,E,T);
	BODY_00_15_epi32(19,T,A,B,C,D,E);

#define BODY_20_39_epi32(i,a,b,c,d,e,f) \
  f.u128 = _mm_add_epi32(stemm[i].u128, e.u128); \
  f.u128 = _mm_add_epi32(f.u128, _mm_rotl_epi32(a.u128, 5)); \
  f.u128 = _mm_add_epi32(f.u128, F_20_39_epi32(b.u128,c.u128,d.u128) ); \
  b.u128 = _mm_rotl_epi32(b.u128, 30); \

	BODY_20_39_epi32(20,E,T,A,B,C,D);
	BODY_20_39_epi32(21,D,E,T,A,B,C);
	BODY_20_39_epi32(22,C,D,E,T,A,B);
	BODY_20_39_epi32(23,B,C,D,E,T,A);
	BODY_20_39_epi32(24,A,B,C,D,E,T);
	BODY_20_39_epi32(25,T,A,B,C,D,E);
	BODY_20_39_epi32(26,E,T,A,B,C,D);
	BODY_20_39_epi32(27,D,E,T,A,B,C);
	BODY_20_39_epi32(28,C,D,E,T,A,B);
	BODY_20_39_epi32(29,B,C,D,E,T,A);
	BODY_20_39_epi32(30,A,B,C,D,E,T);
	BODY_20_39_epi32(31,T,A,B,C,D,E);
	BODY_20_39_epi32(32,E,T,A,B,C,D);
	BODY_20_39_epi32(33,D,E,T,A,B,C);
	BODY_20_39_epi32(34,C,D,E,T,A,B);
	BODY_20_39_epi32(35,B,C,D,E,T,A);
	BODY_20_39_epi32(36,A,B,C,D,E,T);
	BODY_20_39_epi32(37,T,A,B,C,D,E);
	BODY_20_39_epi32(38,E,T,A,B,C,D);
	BODY_20_39_epi32(39,D,E,T,A,B,C);

  //printf("%d a: %x b: %x c: %x d: %x e: %x\n",i , a.u32[0], b.u32[0], c.u32[0], d.u32[0], e.u32[0]);

#define BODY_40_59_epi32(i,a,b,c,d,e,f) \
  f.u128 = _mm_add_epi32(stemm[i].u128, e.u128); \
  f.u128 = _mm_add_epi32(f.u128, _mm_rotl_epi32(a.u128, 5)); \
  f.u128 = _mm_add_epi32(f.u128, F_40_59_epi32(b.u128,c.u128,d.u128)); \
  b.u128 = _mm_rotl_epi32(b.u128, 30); \
 
	BODY_40_59_epi32(40,C,D,E,T,A,B);
	BODY_40_59_epi32(41,B,C,D,E,T,A);
	BODY_40_59_epi32(42,A,B,C,D,E,T);
	BODY_40_59_epi32(43,T,A,B,C,D,E);
	BODY_40_59_epi32(44,E,T,A,B,C,D);
	BODY_40_59_epi32(45,D,E,T,A,B,C);
	BODY_40_59_epi32(46,C,D,E,T,A,B);
	BODY_40_59_epi32(47,B,C,D,E,T,A);
	BODY_40_59_epi32(48,A,B,C,D,E,T);
	BODY_40_59_epi32(49,T,A,B,C,D,E);
	BODY_40_59_epi32(50,E,T,A,B,C,D);
	BODY_40_59_epi32(51,D,E,T,A,B,C);
	BODY_40_59_epi32(52,C,D,E,T,A,B);
	BODY_40_59_epi32(53,B,C,D,E,T,A);
	BODY_40_59_epi32(54,A,B,C,D,E,T);
	BODY_40_59_epi32(55,T,A,B,C,D,E);
	BODY_40_59_epi32(56,E,T,A,B,C,D);
	BODY_40_59_epi32(57,D,E,T,A,B,C);
	BODY_40_59_epi32(58,C,D,E,T,A,B);
	BODY_40_59_epi32(59,B,C,D,E,T,A);
  
#define BODY_60_79_epi32(i,a,b,c,d,e,f) \
  f.u128 = _mm_add_epi32(stemm[i].u128, e.u128); \
  f.u128 = _mm_add_epi32(f.u128, _mm_rotl_epi32(a.u128, 5)); \
  f.u128 = _mm_add_epi32(f.u128, F_60_79_epi32(b.u128,c.u128,d.u128)); \
  b.u128 = _mm_rotl_epi32(b.u128, 30); \

	BODY_60_79_epi32(60,A,B,C,D,E,T);
	BODY_60_79_epi32(61,T,A,B,C,D,E);
	BODY_60_79_epi32(62,E,T,A,B,C,D);
	BODY_60_79_epi32(63,D,E,T,A,B,C);
	BODY_60_79_epi32(64,C,D,E,T,A,B);
	BODY_60_79_epi32(65,B,C,D,E,T,A);
	BODY_60_79_epi32(66,A,B,C,D,E,T);
	BODY_60_79_epi32(67,T,A,B,C,D,E);
	BODY_60_79_epi32(68,E,T,A,B,C,D);
	BODY_60_79_epi32(69,D,E,T,A,B,C);
	BODY_60_79_epi32(70,C,D,E,T,A,B);
	BODY_60_79_epi32(71,B,C,D,E,T,A);
	BODY_60_79_epi32(72,A,B,C,D,E,T);
	BODY_60_79_epi32(73,T,A,B,C,D,E);
	BODY_60_79_epi32(74,E,T,A,B,C,D);
	BODY_60_79_epi32(75,D,E,T,A,B,C);
	BODY_60_79_epi32(76,C,D,E,T,A,B);
	BODY_60_79_epi32(77,B,C,D,E,T,A);
	BODY_60_79_epi32(78,A,B,C,D,E,T);
	BODY_60_79_epi32(79,T,A,B,C,D,E);

//  printf("a: %x b: %x c: %x d: %x e: %x\n", A.u32[0], B.u32[0], C.u32[0], D.u32[0], E.u32[0]);

  vc->h0.u128 = _mm_add_epi32(E.u128, vc->h0.u128);
  vc->h1.u128 = _mm_add_epi32(T.u128, vc->h1.u128);
  vc->h2.u128 = _mm_add_epi32(A.u128, vc->h2.u128);
  vc->h3.u128 = _mm_add_epi32(B.u128, vc->h3.u128);
  vc->h4.u128 = _mm_add_epi32(C.u128, vc->h4.u128);
  return;  
}
