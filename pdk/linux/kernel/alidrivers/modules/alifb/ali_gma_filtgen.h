//+FHDR------------------------------------------------------------------------
/*
	Copyright, ALi
	WARNING: This file contain confidential information of ALi Co., and shall
	not be disclosed to any unauthorized organization or individual.

	Brief:
		scaler filter design function declaration

	Revision history:
		Hudson, 2007-7-9, derived from original C code.
		Hudson, 2007-12-25, add extract_phase() & extract_coef().

	Note:
		the floating point version of functions defined in the file depends on math library.
*/
//-FHDR------------------------------------------------------------------------

#ifndef FILT_GEN_H
#define FILT_GEN_H


// window type definition
#define NO_WIN      0
#define HAMMING_WIN 1
#define LANCZOS_WIN 2

#ifndef WIN_TYPE
#define WIN_TYPE LANCZOS_WIN
#endif


// fixed-fractional type definition
#define FIXED_BITS 32
#define FRAC_BITS 16
#define FRAC_MASK ((1<<FRAC_BITS)-1)
#define INT_BITS (FIXED_BITS-FRAC_BITS)
typedef int frac;

#define FRAC_MAX ((frac)(0x7fffffff))
#define FRAC_MIN ((frac)(0x80000000))
#define FRAC_ONE (1<<FRAC_BITS)
#define FRAC_TWO (FRAC_ONE<<1)
#define FRAC_HALF (FRAC_ONE>>1)
#define FRAC_QUARTER (FRAC_ONE>>2)

#define FLOAT_TO_FRAC(x) ((frac)((x)*FRAC_ONE))
#define DOUBLE_TO_FRAC(x) ((frac)((x)*FRAC_ONE))
#define INT_TO_FRAC(x) ((x)<<FRAC_BITS)
#define FRAC_TO_DOUBLE(x) ((double)(x)/FRAC_ONE)


#ifdef  __cplusplus
extern "C" {
#endif

/*
	designfilter() & designfilterff() are the core functions of this filter design library.
	designfilter() is the floating point version.
	designfilterff() is the fixed-fractional version.
	this two functions do the same thing, design the FIR filter, differ only in the
	  computation in floating or fixed point math.

	FUNCTION PARAMETER:
	srcsampling - the source sample rate
	dstsampling - the target sample rate
	nphase - number of filter phases
	ntap - number of filter taps in each phase
	centered - boolean, whether the filter center should be exactly sampled.
	  if centered is true, the function will design an even order FIR.
	  if centered is false, the function will design an odd order FIR.
	cutoffscale - a factor used to scale the cutoff (-6dB) point, normally it
	  shall be 1.0
	nfracbits - number of bits representing the fractional part of quantized
	  coefficients
	preal - pointer to real coefficients buffer, the length shall be
	  (nphase*ntap + centered)
	pquant - pointer to quantized coefficients buffer, the length shall be
	  (nphase*ntap + centered)

	CAUTION:
	pay attention to the buffer size. if centered is true, the required buffer size
	  is nphase*ntap + 1, NOT nphase*ntap.

	FUNCTION RETURNS:
	none

	COEFFICIENTS LYAOUT IN BUFFER:
	for length n coefficient with nphase phases and ntap taps, where
	  n=nphase*ntap+centered
	let preal/pquant has [c(0) c(1) c(2) c(3) c(4) .... c(n-1)]
	and y = x(0)*tap(0) + x(1)*tap(1) + x(2)*tap(2) + ...
	reorder to multi-phase representation will be
	               |    tap(0)              ...     tap(ntap-2)       tap(ntap-1)
	---------------+--------------------------------------------------------------
	phase(0)       | c(nphase*(ntap-1))              c(nphase)            c(0)
	phase(1)       | c(nphase*(ntap-1)+1)           c(nphase+1)           c(1)
	phase(2)       | c(nphase*(ntap-1)+2)           c(nphase+2)           c(2)
	...            |
	phase(nphase-1)| c(nphase*ntap-1)               c(nphase*2)        c(nphase-1)

	TIPS FOR FILTER DESIGN:
	1. if it's going to make an 1:1 resample filter, the filter must be centered
	2. if it's going to make an upsampling, or scale-up filter, the filter is prefered to
	   be centered
	3. if it's going to make a downsampling, or scale-down filter, it MIGHT be
	   better to design a non-centered filter. however, it's not much better
	   than a centered one. if user want an centered symmetry filter, please refer to
	   item 5.
	   REVISION: to Lanzcos window method, centered filter is good since the window
	   ensure the discarded coefficient is 0. so, ALWAYS design a centered filter.
	4. if the filter response do not satisfy, causing alias or too blurring, try
	   to adjust the cutoff scale factor, which value < 1.0 will attenuate more
	   high frequency component, and value > 1.0 will keep more.
	5. when designing centered filter, the last coefficient of the
	   theoretical impulse response is discarded. user can retrieve this
	   coefficient by taking the first one in the coefficient array. if this
	   coefficient is 0, it's ok to continue. but if it's non-zero, care shall
	   be taken since filter might be attacked by the non-linear phase response.
	   this effect might be significant when the value get rather large according
	   to the sum of all filter coefficients (>=1%).
	   REVISION: designing with Lanzcos window method will eliminate this problem.
	6. always, always, always try to build a longer filter. a long filter can
	   achieve better response, also can help the problem described in item 5.
*/
void designfilter(int srcsampling, int dstsampling, int nphase, int ntap, int centered,
	double cutoffscale, int nfracbits, double *const preal, short *const pquant);
void designfilterff(int srcsampling, int dstsampling, int nphase, int ntap, int centered,
	frac cutoffscale, int nfracbits, frac *const preal, short *const pquant);

void gma_m36f_enhance_coef_gen(int *coef_table, int brightness, int contrast
	, int saturation, int hue, int sharpness);
/*
	extract_phase() & extract_coef() is built to help user ignore the coefficient
	buffer layout from designfilter() & designfilterff().

	PARAMETER:
	coef_t: the target buffer
	coef_s: quantized coefficient buffer from designfilter() & designfilterff()
	nphase: number of phases
	ntap: number of taps
	phase: the phase of coefficients wanted, range in [0, nphase-1]
	tap: the tap of coefficient wanted, range in [0, ntap-1]

	CAUTION:
	user must make sure nphase, ntap, phase, tap are properly specified. these
	functions DO NOT make any boundary check.
 */
void extract_phase(short *coef_t, const short *coef_s, int nphase, int ntap, int phase);
short extract_coef(const short *coef_s, int nphase, int ntap, int phase, int tap);

void vpo_get_hue_sin_cos(unsigned short grade, int *p_sin_val, int *p_cos_val);

#ifdef  __cplusplus
}
#endif

#endif // FILT_GEN_H
