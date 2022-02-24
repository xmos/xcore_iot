// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved

#ifndef DSP_QFORMAT_H_
#define DSP_QFORMAT_H_


/** These Macros can be used to parameterize the conversion macros.
 * E.g.
 * \code
 * #define BP 20  // location of the binary point
 * \endcode
 * Then use the macro like this:
 * \code
 * Q(BP)(1.234567)
 * \endcode
 */
#define F0(N) F ## N
#define F(N) F0(N)

#define Q0(N) Q ## N
#define Q(N) Q0(N)

// Convert from floating point to fixed point Q format.
// The number indicates the fractional bits or the position of the binary point
#define Q31(f) (int)((signed long long)((f) * ((unsigned long long)1 << (31+20)) + (1<<19)) >> 20)
#define Q30(f) (int)((signed long long)((f) * ((unsigned long long)1 << (30+20)) + (1<<19)) >> 20)
#define Q29(f) (int)((signed long long)((f) * ((unsigned long long)1 << (29+20)) + (1<<19)) >> 20)
#define Q28(f) (int)((signed long long)((f) * ((unsigned long long)1 << (28+20)) + (1<<19)) >> 20)
#define Q27(f) (int)((signed long long)((f) * ((unsigned long long)1 << (27+20)) + (1<<19)) >> 20)
#define Q26(f) (int)((signed long long)((f) * ((unsigned long long)1 << (26+20)) + (1<<19)) >> 20)
#define Q25(f) (int)((signed long long)((f) * ((unsigned long long)1 << (25+20)) + (1<<19)) >> 20)
#define Q24(f) (int)((signed long long)((f) * ((unsigned long long)1 << (24+20)) + (1<<19)) >> 20)
#define Q23(f) (int)((signed long long)((f) * ((unsigned long long)1 << (23+20)) + (1<<19)) >> 20)
#define Q22(f) (int)((signed long long)((f) * ((unsigned long long)1 << (22+20)) + (1<<19)) >> 20)
#define Q21(f) (int)((signed long long)((f) * ((unsigned long long)1 << (21+20)) + (1<<19)) >> 20)
#define Q20(f) (int)((signed long long)((f) * ((unsigned long long)1 << (20+20)) + (1<<19)) >> 20)
#define Q19(f) (int)((signed long long)((f) * ((unsigned long long)1 << (19+20)) + (1<<19)) >> 20)
#define Q18(f) (int)((signed long long)((f) * ((unsigned long long)1 << (18+20)) + (1<<19)) >> 20)
#define Q17(f) (int)((signed long long)((f) * ((unsigned long long)1 << (17+20)) + (1<<19)) >> 20)
#define Q16(f) (int)((signed long long)((f) * ((unsigned long long)1 << (16+20)) + (1<<19)) >> 20)
#define Q15(f) (int)((signed long long)((f) * ((unsigned long long)1 << (15+20)) + (1<<19)) >> 20)
#define Q14(f) (int)((signed long long)((f) * ((unsigned long long)1 << (14+20)) + (1<<19)) >> 20)
#define Q13(f) (int)((signed long long)((f) * ((unsigned long long)1 << (13+20)) + (1<<19)) >> 20)
#define Q12(f) (int)((signed long long)((f) * ((unsigned long long)1 << (12+20)) + (1<<19)) >> 20)
#define Q11(f) (int)((signed long long)((f) * ((unsigned long long)1 << (11+20)) + (1<<19)) >> 20)
#define Q10(f) (int)((signed long long)((f) * ((unsigned long long)1 << (10+20)) + (1<<19)) >> 20)
#define Q9(f)  (int)((signed long long)((f) * ((unsigned long long)1 << (9+20)) + (1<<19)) >> 20)
#define Q8(f)  (int)((signed long long)((f) * ((unsigned long long)1 << (8+20)) + (1<<19)) >> 20)

// Convert from fixed point to double precision floating point
// The number indicates the fractional bits or the position of the binary point
#define F31(x) ((double)(x)/(double)(uint32_t)(1<<31)) // needs uint32_t cast because bit 31 is 1
#define F30(x) ((double)(x)/(double)(1<<30))
#define F29(x) ((double)(x)/(double)(1<<29))
#define F28(x) ((double)(x)/(double)(1<<28))
#define F27(x) ((double)(x)/(double)(1<<27))
#define F26(x) ((double)(x)/(double)(1<<26))
#define F25(x) ((double)(x)/(double)(1<<25))
#define F24(x) ((double)(x)/(double)(1<<24))
#define F23(x) ((double)(x)/(double)(1<<23))
#define F22(x) ((double)(x)/(double)(1<<22))
#define F21(x) ((double)(x)/(double)(1<<21))
#define F20(x) ((double)(x)/(double)(1<<20))
#define F19(x) ((double)(x)/(double)(1<<19))
#define F18(x) ((double)(x)/(double)(1<<18))
#define F17(x) ((double)(x)/(double)(1<<17))
#define F16(x) ((double)(x)/(double)(1<<16))

// short
#define F15(x) ((double)(x)/(double)(1<<15))
#define F14(x) ((double)(x)/(double)(1<<14))
#define F13(x) ((double)(x)/(double)(1<<13))
#define F12(x) ((double)(x)/(double)(1<<12))
#define F11(x) ((double)(x)/(double)(1<<11))
#define F10(x) ((double)(x)/(double)(1<<10))
#define F9(x)  ((double)(x)/(double)(1<<9))
#define F8(x)  ((double)(x)/(double)(1<<8))
#endif

