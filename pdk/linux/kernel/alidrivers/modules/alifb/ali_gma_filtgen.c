//+FHDR------------------------------------------------------------------------
/*
	Copyright, ALi
	WARNING: This file contain confidential information of ALi Co., and shall
	not be disclosed to any unauthorized organization or individual.

	BRIEF:
		scaler filter design function

	REVISION HISTORY:
		Hudson, 2006-3-1, create file
		Hudson, 2007-6-12, add Lanczos window method
		Hudson, 2007-7-9, separate declaration to header file, move to visual prototype repository
		Hudson, 2007-7-30, modify tunefilter()
		Hudson, 2007-8-6, change build platform default to cygwin
		Hudson, 2007-12-25, add extract_phase() & extract_coef(). modify tunefilter().

	NOTE:
		the floating point version of functions defined in the file depends on math library.

	BUILD:
		user can build a stand-alone executable from this file. for example, using VC6, user can build it by
			cl filtgen.c /DDOUBLE_VER /DFILT_UNITTEST /DWIN32
		for Cygwin environment, no extra define is required to build as a library.
*/
//-FHDR------------------------------------------------------------------------

#ifndef CONFIG_RPC_HLD_GMA

#include "ali_gma_filtgen.h"

//#if defined(DUAL_ENABLE) && defined(MAIN_CPU)

#define ENTRY_BITS 8

#define PI 3.141592653589793238462643383279502884197169399375105820974944592
#define INV_PI (1/PI)

#define FRAC_PI (DOUBLE_TO_FRAC(PI))
#define FRAC_INV_PI (DOUBLE_TO_FRAC(INV_PI))

#ifdef WIN32
typedef __int64 frac_product;
#define INLINE __inline
/*
typedef int acc_hi;
typedef unsigned int acc_lo;
typedef struct {
	acc_hi hi;
	acc_lo lo;
} frac_acc;

static INLINE
void madd(frac a, frac b, frac_acc *c) {
	frac_product z = c->hi;
	z <<= FIXED_BITS;
	z |= c->lo;
	z += (frac_product)a * b;
	c->hi = (acc_hi)(z >> FIXED_BITS);
	c->lo = (acc_lo)(z);
}
*/
#else
typedef long long frac_product;
#define INLINE __inline__
/*
INLINE
frac mul(frac x, frac y) {
	frac hi, lo;
	asm ("mult %2, %3" : "=l" (lo), "=h" (hi) : "%r" (x), "r" (y));
	return hi << (N_FIXED_BITS - N_FRAC_BITS);
}
*/
#endif

const char filtgen_win_msg[] =
	"FiltGen "
    #if (WIN_TYPE == HAMMING_WIN)
    "Hamming"
    #elif (WIN_TYPE == LANCZOS_WIN)
    "Lanczos"
    #else
    "No Win"
    #endif
    ;

#if (WIN_TYPE == HAMMING_WIN)
#define WIN_FUNC(len, idx) (hamming(len, idx))
#define WIN_FUNC_FF(len, idx) (hammingff(len, idx))
#elif (WIN_TYPE == LANCZOS_WIN)
#define WIN_FUNC(len, idx) (lanczos(len, idx))
#define WIN_FUNC_FF(len, idx) (lanczosff(len, idx))
#else
#define WIN_FUNC(len, idx) (1)
#define WIN_FUNC_FF(len, idx) (FRAC_ONE)
#endif

// adjust quantized coefficients to make sum of each phase is 1.0
static
void tunefilter(int nphase, int ntap, int scale, short *const pquant)
{
	int i, j;
	const int ref_tap = ntap >> 1;
	for (i=0; i<nphase; i++) {
		int sum = 0;
		short *pq = pquant + i;
		short *pq_ref = pq + ref_tap*nphase;
		short *pq_adj = pq + (ref_tap-1)*nphase;
		for (j=0; j<ntap; j++) {
			sum += *pq;
			pq += nphase;
		}
		if (sum != scale) {
			int adjustment = scale - sum;
			/*  central 2 taps always be the greatest. in case of odd taps,
			    central tap is the greatest.
			 */
			if (*pq_ref > *pq_adj) {
				*pq_ref += adjustment;
			} else if (*pq_ref < *pq_adj) {
				*pq_adj += adjustment;
			} else {
				*pq_ref += (adjustment+1)/2;
				*pq_adj += adjustment/2;
			}
		}
		/* // maybe this one is not good, modify to next to it. 2007-12-25
		if (i != 0) {
			if (*pq_ref >= scale) {
				int adjustment = *pq_ref - scale + 1;
				*pq_ref -= adjustment;
				*pq_adj += adjustment;
			} else if (*pq_adj >= scale) {
				int adjustment = *pq_adj - scale + 1;
				*pq_adj -= adjustment;
				*pq_ref += adjustment;
			}
		} */
		// hudson, 2007-12-25
		if (*pq_ref > scale) {
			int adjustment = *pq_ref - scale;
			*pq_ref -= adjustment;
			*pq_adj += adjustment;
		} else if (*pq_adj > scale) {
			int adjustment = *pq_adj - scale;
			*pq_adj -= adjustment;
			*pq_ref += adjustment;
		}
	}
}


#ifdef DOUBLE_VER

#include <math.h>

static
double sinc(double x)
{
	if (x == 0) {
		return 1.0;
	} else {
		return sin(PI*x) / (PI*x);
	}
}

/*
    design fir using least-square method
*/
static
void firls(double *const pcoef, const double cutoff, const int len)
{
	int i, s;
	double b0=0, b1, m, k;
	double *b; // b(x)
	int oddlen = len % 2;
	int halflen = len/2;
	double amp[4] = {1, 1, 0, 0};
	double bb[4] = {0, 0, 0, 1.0/2.0}; // band bound/2
	bb[1] = bb[2] = cutoff/2.0;
	for (i=0; i<len; i++) {
		pcoef[i] = 0;
	}
	b = pcoef + len/2 + (oddlen ? 1 : 0);
	for (s=0; s<sizeof(bb)/sizeof(bb[0]); s+=2) {
		m = (amp[s+1] - amp[s]) / (bb[s+1] - bb[s]);
		b1 = amp[s] - m * bb[s];
		if (oddlen) {
			b0 += b1 * (bb[s+1] - bb[s]) + m / 2 * (bb[s+1]*bb[s+1] - bb[s]*bb[s]);
		}
		if (oddlen) {
			for (i=0; i<halflen; i++) {
				k = (double)i + 1;
				b[i] += m/(4*PI*PI)*(cos(2*PI*k*bb[s+1])-cos(2*PI*k*bb[s])) / (k*k);
				b[i] += bb[s+1]*(m*bb[s+1]+b1)*sinc(2*k*bb[s+1]) - bb[s]*(m*bb[s]+b1)*sinc(2*k*bb[s]);
			}
		} else {
			for (i=0; i<halflen; i++) {
				k = (double)i + 0.5;
				b[i] += m/(4*PI*PI)*(cos(2*PI*k*bb[s+1])-cos(2*PI*k*bb[s])) / (k*k);
				b[i] += bb[s+1]*(m*bb[s+1]+b1)*sinc(2*k*bb[s+1]) - bb[s]*(m*bb[s]+b1)*sinc(2*k*bb[s]);
			}
		}
	}
	if (oddlen) {
		pcoef[len/2] = b0;
		for (i=0; i<halflen; i++) {
			pcoef[len/2-1-i] = pcoef[len/2+1+i];
		}
	} else {
		for (i=0; i<halflen; i++) {
			pcoef[(len-1)/2-i] = pcoef[len/2+i];
		}
	}
	for (i=0; i<len; i++)
		pcoef[i] *= 2;
}

/*
    calculate hamming window
      n - window length
      k - position in the window
*/
static
double hamming(int n, int k)
{
	return 0.54 - 0.46 * cos(2 * PI * k / (n-1));
}

/*
    calculate Lanczos window
      n - window length
      k - position in the window, [0, n-1]
*/
static
double lanczos(int n, int k)
{
	double x = (double)(k*2)/(n-1) - 1;
	double y = sinc(x);
	//fprintf(stderr, "[float] n=%3d, k=%3d, x=%20g, y=%20g\n", n, k, x, y);
	return y;
}

/*
    convergent rounding, rounds to the nearest integer, except in a tie, then
    round to the nearest even integer
*/
static
int convergent(double x)
{
	double intpart;
	double fracpart = modf(x, &intpart);
	if (fracpart>0.5 || (fracpart==0.5 && (int)(intpart)%2!=0)) {
		intpart += 1;
	} else if (fracpart<-0.5 || (fracpart==-0.5 && (int)(-intpart)%2!=0)) {
		intpart -= 1;
	}
	return (int)(intpart);
}

// normalize sum of each phase to 1.0
// phases of filter kernel designed by firls do not have a unique sum.
//   normalize them separately may lead to unexpected result, e.g., the
//   central tap is not the greatest. this will be compensated by tunefilter()
static
void normalize(int nphase, int ntap, double *const preal)
{
	int i, j;
	double *prcoef, sum;
	for (i=0; i<nphase; i++) {
		sum = 0;
		prcoef = preal + i;
		for (j=0; j<ntap; j++) {
			sum += *prcoef;
			prcoef += nphase;
		}
		prcoef = preal + i;
		for (j=0; j<ntap; j++) {
			*prcoef /= sum;
			prcoef += nphase;
		}
	}
}

static
void quantize(short *pquant, double *preal, int len, int scale)
{
	int i;
	for (i=0; i<len; i++) {
		*pquant++ = convergent(*preal++ * scale);
	}
}

void designfilter(int srcsampling, int dstsampling, int nphase, int ntap, int centered,
	double cutoffscale, int nfracbits, double *const preal, short *const pquant)
{
	int i, filtlen, scale;
	double cutoff;
	filtlen = nphase * ntap + centered;
	if (dstsampling >= srcsampling) { // scaleup
		cutoff = 1.0 / nphase;
	} else { // scaledown
		cutoff = 1.0 / nphase * ((double)(dstsampling) / srcsampling);
	}
	// design filter
	firls(preal, cutoff*cutoffscale, filtlen);
	// window by hamming
	// window by rectangle leads ringing
	for (i=0; i<filtlen; i++) {
		preal[i] *= WIN_FUNC(filtlen, i);
	}
	normalize(nphase, ntap, preal);
	scale = 1 << nfracbits;
	quantize(pquant, preal, filtlen, scale);
	tunefilter(nphase, ntap, scale, pquant);
}

#endif // DOUBLE_VER




static INLINE
frac mul(frac a, frac b) {
	frac_product c = a;
	c *= b;
	return (frac)(c>>FRAC_BITS);
}

static INLINE
frac mul_frac_int(frac a, int b) {
	return a*b;
}

static const
unsigned short cos_lut[1<<ENTRY_BITS] = {
	65535,
	65531,
	65525,
	65516,
	65505,
	65492,
	65476,
	65457,
	65436,
	65413,
	65387,
	65358,
	65328,
	65294,
	65259,
	65220,
	65180,
	65137,
	65091,
	65043,
	64993,
	64940,
	64884,
	64827,
	64766,
	64704,
	64639,
	64571,
	64501,
	64429,
	64354,
	64277,
	64197,
	64115,
	64031,
	63944,
	63854,
	63763,
	63668,
	63572,
	63473,
	63372,
	63268,
	63162,
	63054,
	62943,
	62830,
	62714,
	62596,
	62476,
	62353,
	62228,
	62101,
	61971,
	61839,
	61705,
	61568,
	61429,
	61288,
	61145,
	60999,
	60851,
	60700,
	60547,
	60392,
	60235,
	60075,
	59914,
	59750,
	59583,
	59415,
	59244,
	59071,
	58896,
	58718,
	58538,
	58356,
	58172,
	57986,
	57798,
	57607,
	57414,
	57219,
	57022,
	56823,
	56621,
	56418,
	56212,
	56004,
	55794,
	55582,
	55368,
	55152,
	54934,
	54714,
	54491,
	54267,
	54040,
	53812,
	53581,
	53349,
	53114,
	52878,
	52639,
	52398,
	52156,
	51911,
	51665,
	51417,
	51166,
	50914,
	50660,
	50404,
	50146,
	49886,
	49624,
	49361,
	49095,
	48828,
	48559,
	48288,
	48015,
	47741,
	47464,
	47186,
	46906,
	46624,
	46341,
	46056,
	45769,
	45480,
	45190,
	44898,
	44604,
	44308,
	44011,
	43713,
	43412,
	43110,
	42806,
	42501,
	42194,
	41886,
	41576,
	41264,
	40951,
	40636,
	40320,
	40002,
	39683,
	39362,
	39040,
	38716,
	38391,
	38064,
	37736,
	37407,
	37076,
	36744,
	36410,
	36075,
	35738,
	35401,
	35062,
	34721,
	34380,
	34037,
	33692,
	33347,
	33000,
	32652,
	32303,
	31952,
	31600,
	31248,
	30893,
	30538,
	30182,
	29824,
	29466,
	29106,
	28745,
	28383,
	28020,
	27656,
	27291,
	26925,
	26558,
	26190,
	25821,
	25451,
	25080,
	24708,
	24335,
	23961,
	23586,
	23210,
	22834,
	22457,
	22078,
	21699,
	21320,
	20939,
	20557,
	20175,
	19792,
	19409,
	19024,
	18639,
	18253,
	17867,
	17479,
	17091,
	16703,
	16314,
	15924,
	15534,
	15143,
	14751,
	14359,
	13966,
	13573,
	13180,
	12785,
	12391,
	11996,
	11600,
	11204,
	10808,
	10411,
	10014,
	 9616,
	 9218,
	 8820,
	 8421,
	 8022,
	 7623,
	 7224,
	 6824,
	 6424,
	 6023,
	 5623,
	 5222,
	 4821,
	 4420,
	 4019,
	 3617,
	 3216,
	 2814,
	 2412,
	 2010,
	 1608,
	 1206,
	  804,
	  402,
	    0,
};

// accept pi-based normalized radius
// cosine, fixed-point fractional normalized
static
frac cosffn(frac x) {
	int neg = 0;
	int index, alpha;
	const int residue_nbit = FRAC_BITS-(ENTRY_BITS+1);
	frac prev_val, next_val, val;
	x = x<0 ? -x : x;
	// fold to [0, 2)*pi
	x &= (2<<FRAC_BITS) - 1;
	// fold to [0..1)*pi
	if (x >= FRAC_ONE)
		x = FRAC_TWO - x;
	// fold x to [0..0.5]*pi
	if (x > FRAC_HALF) {
		x = FRAC_ONE - x;
		neg = 1;
	}
	index = x>>residue_nbit;
	alpha = x & ((1<<residue_nbit)-1);
	//printf("--%8x %d", x, index);
	prev_val = index==0 ? FRAC_ONE : cos_lut[index-1];
	next_val = index==(1<<ENTRY_BITS) ? -cos_lut[index-2] : cos_lut[index];
	val = (prev_val*((1<<residue_nbit)-alpha)+next_val*alpha+(1<<(residue_nbit-1))) >> residue_nbit;
	return neg ? -val : val;
}

static
frac sinffn(frac x) {
	return cosffn(FRAC_HALF - x);
}

static
frac reciprocal(frac x) {
	int neg = x < 0;
	int i, n=2;
	frac y;//reciprocal_lut[(x>>(FRAC_BITS-5))&((1<<10)-1)];
	unsigned mask = 0x40000000;
	int count=1, e;
	if (x<0)
		x = -x;
	while ((mask & x) == 0) {
		mask >>= 1;
		count++;
		if (count==32)
			return neg ? FRAC_MIN : FRAC_MAX;
	}
	e = FRAC_BITS - count;
	//printf("count=%d e=%d x=%08x ->", count, e, x);
	// normalize to [0.5, 1)
	if (e>0) {
		x >>= e;
	} else {
		x <<= -e;
	}
	//printf("%08x\n", x);
	y = DOUBLE_TO_FRAC(2.9282032302755091741)-(x<<1);
	// y = y*(2-x*y)
	for (i=0; i<n; i++)
		y = mul(y, FRAC_TWO-mul(x, y));
	y = e>0 ? y>>e : y<<-e;
	return neg ? -y : y;
}

/*
void init_reciprocal_table() {
	frac x;
	int i;
	reciprocal_lut[0] = FRAC_MAX;
	reciprocal_lut[1023] = 0;
	x = FRAC_ONE >> 5;
	for (i=1; i<sizeof(reciprocal_lut)/sizeof(reciprocal_lut[0]-1); i++) {
		reciprocal_lut[i] = (frac)((FRAC_ONE/(double)x)*FRAC_ONE);
		//printf("%g\n", (double)reciprocal_lut[i]/FRAC_ONE);
		x += FRAC_ONE>>5;
	}
}
*/

// sin(PI*x) / (PI*x);
static
frac sincff(frac x) {
	if (x == 0) {
		return FRAC_ONE;
	} else {
		return mul(sinffn(x), mul(reciprocal(x), FRAC_INV_PI));
	}
}

#define FRAC_2PI_SQ DOUBLE_TO_FRAC(4*PI*PI)
#define FRAC_INV_2PI_SQ DOUBLE_TO_FRAC(1/(4*PI*PI))
/*
    design fir using least-square method
*/
static
void firlsff(frac *const pcoef, const frac cutoff, const int len)
{
	int i;
	unsigned s;
	frac b0=0, b1, m, k;
	frac *b; // b(x)
	int oddlen = len % 2;
	int halflen = len/2;
	frac amp[4] = {FRAC_ONE, FRAC_ONE, 0, 0};
	frac bb[4] = {0, 0, 0, FRAC_HALF}; // band bound/2
	bb[1] = bb[2] = cutoff>>1;
	for (i=0; i<len; i++) {
		pcoef[i] = 0;
	}
	b = pcoef + len/2 + (oddlen ? 1 : 0);
	for (s=0; s<sizeof(bb)/sizeof(bb[0]); s+=2) {
		//m = DOUBLE_TO_FRAC(FRAC_TO_DOUBLE(amp[s+1] - amp[s]) / FRAC_TO_DOUBLE(bb[s+1] - bb[s]));
		m = mul((amp[s+1] - amp[s]), reciprocal(bb[s+1] - bb[s]));
		b1 = amp[s] - mul(m,bb[s]);
		if (oddlen) {
			b0 += mul(b1,(bb[s+1]-bb[s])) + mul((m>>1),(mul(bb[s+1],bb[s+1])-mul(bb[s],bb[s])));
		}
		for (i=0; i<halflen; i++) {
			frac t0, t1, t2;
			k = INT_TO_FRAC(i) + (oddlen?FRAC_ONE:FRAC_HALF);
			//t0 = DOUBLE_TO_FRAC(cos(2*PI*FRAC_TO_DOUBLE(mul(k,bb[s+1])))-cos(2*PI*FRAC_TO_DOUBLE(mul(k,bb[s]))));
			t0 = cosffn(mul(k,bb[s+1])<<1)-cosffn(mul(k,bb[s])<<1);
			//printf("+ %d\n", t0);
			//t1 = mul(mul(bb[s+1],(mul(m,bb[s+1])+b1)),DOUBLE_TO_FRAC(sinc(FRAC_TO_DOUBLE(mul(2*k,bb[s+1])))));
			t1 = mul(mul(bb[s+1],(mul(m,bb[s+1])+b1)), sincff(mul(2*k,bb[s+1])));
			//t2 = mul(mul(bb[s],(mul(m,bb[s])+b1)),DOUBLE_TO_FRAC(sinc(FRAC_TO_DOUBLE(mul(2*k,bb[s])))));
			t2 = mul(mul(bb[s],(mul(m,bb[s])+b1)), sincff(mul(2*k,bb[s])));
			b[i] += mul(mul(mul(m,FRAC_INV_2PI_SQ),t0), reciprocal(mul(k,k)));
			b[i] += t1 - t2;
			//printf("- b[%d]=%g, %x, %x, %x\n", i, FRAC_TO_DOUBLE(b[i]), mul(mul(mul(m,FRAC_INV_2PI_SQ),t0),DOUBLE_TO_FRAC(1/FRAC_TO_DOUBLE(mul(k,k)))), t0, t1);
		}
	}
	if (oddlen) {
		pcoef[len/2] = b0;
		for (i=0; i<halflen; i++) {
			pcoef[len/2-1-i] = pcoef[len/2+1+i];
		}
	} else {
		for (i=0; i<halflen; i++) {
			pcoef[(len-1)/2-i] = pcoef[len/2+i];
		}
	}
	for (i=0; i<len; i++)
		pcoef[i] <<= 1;
}

#if (WIN_TYPE == HAMMING_WIN)
/*
    calculate hamming window
      n - window length
      k - position in the window
    formula: 0.54 - 0.46 * cos(2 * PI * k / (n-1))
*/
static
frac hammingff(int n, int k)
{
	static int last_n = 0;
	static frac den = 0;
	if (last_n != n) {
		last_n = n;
		den = reciprocal(INT_TO_FRAC(n-1));
		//printf("---%f\n", FRAC_TO_DOUBLE(den));
	}
	return DOUBLE_TO_FRAC(0.54) - mul(DOUBLE_TO_FRAC(0.46), cosffn(mul(INT_TO_FRAC(k)<<1, den)));
}

#elif (WIN_TYPE == LANCZOS_WIN)
/*
    calculate Lanczos window
      n - window length
      k - position in the window, [0, n-1]
*/
static
frac lanczosff(int n, int k)
{
	static int last_n = 0;
	static frac den = 0;
	frac x, y;
	if (last_n != n) {
		last_n = n;
		den = reciprocal(INT_TO_FRAC(n-1));
		//printf("---%f\n", FRAC_TO_DOUBLE(den));
	}
	x = mul(INT_TO_FRAC(k<<1), den) - INT_TO_FRAC(1);
	y = sincff(x);
	//fprintf(stderr, "[fixed] n=%3d, k=%3d, x=%20g, y=%20g\n", n, k, FRAC_TO_DOUBLE(x), FRAC_TO_DOUBLE(y));
	return y;
}

#endif

/*
    convergent rounding, rounds to the nearest integer, except in a tie, then
    round to the nearest even integer
*/
static
int convergentff(frac x)
{
	int neg = x<0;
	int intpart, fracpart;
	x = x>0 ? x : -x;
	intpart = x >> FRAC_BITS;
	fracpart = x & FRAC_MASK;
	if (fracpart>FRAC_HALF || ((fracpart==FRAC_HALF) && ((intpart&1)!=0)))
		intpart++;
	return neg ? -intpart : intpart;
}

static
void quantizeff(short *pquant, frac *preal, int len, int nfracbits)
{
	int i;
	for (i=0; i<len; i++) {
		*pquant++ = convergentff(*preal++ << nfracbits);
	}
}

// normalize sum of each phase to 1.0
static
void normalizeff(int nphase, int ntap, frac *const preal)
{
	int i, j;
	frac *prcoef, sum, a;
	for (i=0; i<nphase; i++) {
		sum = 0;
		prcoef = preal + i;
		for (j=0; j<ntap; j++) {
			sum += *prcoef;
			prcoef += nphase;
		}
		prcoef = preal + i;
		a = reciprocal(sum);
		//printf("sum=%f, r=%f, ref=%f, diff=%f\n", FRAC_TO_DOUBLE(sum), FRAC_TO_DOUBLE(a), 1/FRAC_TO_DOUBLE(sum), fabs(FRAC_TO_DOUBLE(a)- 1/FRAC_TO_DOUBLE(sum)));
		for (j=0; j<ntap; j++) {
			*prcoef = mul(*prcoef, a);//*prcoef /= sum;
			prcoef += nphase;
		}
	}
}

void designfilterff(int srcsampling, int dstsampling, int nphase, int ntap, int centered,
	frac cutoffscale, int nfracbits, frac *const preal, short *const pquant)
{
	int i, filtlen, scale;
	frac cutoff;
	filtlen = nphase * ntap + centered;
	if (dstsampling >= srcsampling) { // scaleup
		cutoff = reciprocal(INT_TO_FRAC(nphase));
	} else { // scaledown
		cutoff = mul(reciprocal(INT_TO_FRAC(nphase)), mul_frac_int(reciprocal(INT_TO_FRAC(srcsampling)), dstsampling));
	}
	cutoff = mul(cutoff, cutoffscale);
	//printf("cutoff=%f\n", FRAC_TO_DOUBLE(cutoff));
	// design filter
	firlsff(preal, cutoff, filtlen);
	// windowing
	for (i=0; i<filtlen; i++) {
		preal[i] = mul(preal[i], WIN_FUNC_FF(filtlen, i));
	}
	normalizeff(nphase, ntap, preal);
	scale = 1 << nfracbits;
	quantizeff(pquant, preal, filtlen, nfracbits);
	tunefilter(nphase, ntap, scale, pquant);
}

void extract_phase(short *coef_t, const short *coef_s, int nphase, int ntap, int phase) {
	int i;
	coef_t += ntap - 1;
	coef_s += phase;
	for (i=0; i<ntap; i++) {
		*coef_t-- = *coef_s;
		coef_s += nphase;
	}
}

short extract_coef(const short *coef_s, int nphase, int ntap, int phase, int tap) {
	return *(coef_s + nphase*(ntap-1-tap) + phase);
}

#ifdef FILT_UNITTEST

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static
void init_cos_table() {
	int i;
	int nentry = 1<<ENTRY_BITS;
	double step = 0.5*PI / nentry;
	double p = step;
	for (i=0; i<nentry; i++) {
		frac value = (frac)(cos(p)*(1<<FRAC_BITS) + 0.5);
		//printf("\t%5d,\n", value);
		assert(cos_lut[i] == value);
		p += step;
	}
}

#if 0
void test_cosffn(int n) {
	frac r, badguy;
	frac step = 1;//FRAC_HALF / (1<<ENTRY_BITS);
	double maxdiff=0;
	int count=0;
	for (r=0; r<n; r+=1) {
		//printf("%8x, %d\n", r, cosffn(r));
		double value, valref, diff;
		value = FRAC_TO_DOUBLE(cosffn(DOUBLE_TO_FRAC((double)r/(n-1))));
		valref = cos((double)r/(n-1) * PI);
		//printf("%8g, %8g\n", value, valref);
		diff = fabs(valref-value);
		if (diff > maxdiff) {
			maxdiff = diff;
			badguy = r;
		}
		count++;
	}
	printf("max abs diff=%g, bad guy=%d, %d tested\n", maxdiff, badguy, count);
}

void test_sinffn() {
	frac r;
	frac step = 1;//FRAC_HALF / (1<<ENTRY_BITS);
	double maxdiff=0;
	int count=0;
	for (r=-FRAC_TWO*2; r<FRAC_TWO*2; r+=step) {
		//printf("%8x, %d\n", r, cosffn(r));
		double value, valref, diff;
		value = (double)sinffn(r) / (1<<FRAC_BITS);
		valref = sin(((double)r+0.9999)/FRAC_ONE * PI);
		//printf("%8g, %8g\n", value, valref);
		diff = fabs(valref-value);
		if (diff > maxdiff)
			maxdiff = diff;
		count++;
	}
	printf("max abs diff = %g, %d tested\n", maxdiff, count);
}

void test_sine(int argc, char *argv[]) {
	frac x;
	double dx, dy, diff;
	test_cosffn(atoi(argv[1]));
	test_sinffn();
}

void test_reciprocal2() {
	double d, dr, maxdiff=0, diff;
	frac fr;
	int count=0;
	for (d=-10; d<10; d+=0.3) {
		fr = reciprocal((frac)(d*FRAC_ONE));
		dr = 1/d;
		diff = fabs((double)fr/FRAC_ONE - dr);
		printf("d=%g, fr=%g, dr=%g, diff=%g\n", d, (double)fr/FRAC_ONE, dr, diff);
		if (diff > maxdiff)
			maxdiff = diff;
		count++;
	}
	printf("reciprocal test: %d number tested, max diff=%g\n", count, maxdiff);
}

#define NTAP 32*8+1
void test_ls(int argc, char *argv[]) {
	double hd[NTAP], maxdiff, cutoff;
	frac hf[NTAP];
	int i, ntap;
	assert(argc==3);
	ntap = atoi(argv[1]);
	cutoff = atof(argv[2]);
	firls(hd, cutoff, ntap);
	firlsff(hf, DOUBLE_TO_FRAC(cutoff), ntap);
	for (i=0; i<ntap; i++) {
		double hfd = FRAC_TO_DOUBLE(hf[i]);
		double diff = fabs(hfd-hd[i]);
		printf("[%4d] %8.4f %8.4f diff=%8.3e\n", i, hd[i], hfd, diff);
		if (diff > maxdiff)
			maxdiff = diff;
	}
	printf("maxdiff=%8.3e [%d]\n", maxdiff, (int)(maxdiff*256));
}

void test_reciprocal(int argc, char *argv[]) {
	frac x;
	double dx, dy, diff;
	assert(argc==2);
	//init_reciprocal_table();
	//test_reciprocal();
	x = DOUBLE_TO_FRAC(atof(argv[1]));
	dx = (double)x/FRAC_ONE;
	dy = (double)reciprocal(x)/FRAC_ONE;
	diff = fabs(1/dx-dy);
	printf("x=%g, get=%g, ref=%g, diff=%g (%3.2f%)\n", dx, dy, 1/dx, diff, diff*100*dx);
}

void test_hamming(int argc, char *argv[]) {
	int i, n;
	double maxdiff=0, diff, a, b;
	assert(argc==2);
	n = atoi(argv[1]);
	for (i=0; i<n; i++) {
		a = hamming(n, i);
		b = FRAC_TO_DOUBLE(hammingff(n, i));
		diff = fabs(a-b);
		printf("reg=%g get=%g diff=%g\n", a, b, diff);
		if (diff > maxdiff)
			maxdiff = diff;
	}
	printf("maxdiff=%g\n", maxdiff);
}

void test_convergent() {
	double vec[] = {
		0, 1.1, 1.125, 1.26, 1.49999, 1.5, 1.511111, 2.5, 2.501
	};
	int i;
	for (i=0; i<sizeof(vec)/sizeof(vec[0]); i++) {
		int ref = convergent(vec[i]);
		int a = convergentff(DOUBLE_TO_FRAC(vec[i]));
		printf("%g -> ref=%d, get=%d, diff=%d\n", vec[i], ref, a, a-ref);
		vec[i] = -vec[i];
	}
	for (i=0; i<sizeof(vec)/sizeof(vec[0]); i++) {
		int ref = convergent(vec[i]);
		int a = convergentff(DOUBLE_TO_FRAC(vec[i]));
		printf("%g -> ref=%d, get=%d, diff=%d\n", vec[i], ref, a, a-ref);
	}
}

int main___test(int argc, char *argv[]) {
	init_cos_table();
	//test_hamming(argc, argv);
	//test_convergent();
	//test_ls(argc, argv);
	test_designfilter(argc, argv);
	return 0;
}

#endif

int main(int argc, char* argv[])
{
	static double coef[32*8+1];
	static short quant[32*8+1];
	static frac coef2[32*8+1];
	static short quant2[32*8+1];
	int nphase, ntap, src, dst, centered, imaxdiff=0;
	double cutoffscale, dmaxdiff=0;
	int i, j;
	if (argc != 7) {
		fprintf(stderr, "filtgen <src> <dst> <nphase> <ntap> <centered> <cutoffscale>\n");
		exit(-1);
	}
	printf("design filter with %s window\n", filtgen_win_msg);
	init_cos_table(); // !!!!!!!!!!!!!!!!!
	src = strtol(argv[1], NULL, 0);
	dst = strtol(argv[2], NULL, 0);
	nphase = strtol(argv[3], NULL, 0);
	ntap = strtol(argv[4], NULL, 0);
	if (strcmp(argv[5], "yes") == 0) {
		centered = 1;
	} else if (strcmp(argv[5], "no") == 0) {
		centered = 0;
	} else {
		fprintf(stderr, "must be 'yes' or 'no'\n");
		exit(-1);
	}
	cutoffscale = atof(argv[6]);
	assert(ntap <= 8);
	designfilter(src, dst, nphase, ntap, centered, cutoffscale, 8, coef, quant);
	designfilterff(src, dst, nphase, ntap, centered, DOUBLE_TO_FRAC(cutoffscale), 8, coef2, quant2);
	for (i=0; i<nphase; i++) {
		for (j=0; j<ntap; j++) {
			double diff = fabs(coef[j*nphase+i]-FRAC_TO_DOUBLE(coef2[j*nphase+i]));
			if (diff > dmaxdiff)
				dmaxdiff = diff;
		}
	}
	for (i=0; i<nphase; i++) {
		for (j=0; j<ntap; j++) {
			int diff = labs(quant[j*nphase+i]-quant2[j*nphase+i]);
			if (diff > imaxdiff)
				imaxdiff = diff;
		}
	}
	printf("====== double version quantized, reversed order ======\n");
	for (i=0; i<nphase; i++) {
		for (j=0; j<ntap; j++) {
			printf("%4d,", extract_coef(quant, nphase, ntap, i, j));
		}
		printf("\n");
	}
	printf("====== fixed version quantized, reversed order ======\n");
	for (i=0; i<nphase; i++) {
		for (j=0; j<ntap; j++) {
			printf("%4d,", extract_coef(quant2, nphase, ntap, i, j));
		}
		printf("\n");
	}
	printf("max diff:  floating=%g, fixed=%d\n", dmaxdiff, imaxdiff);

	return 0;
}

void fw_sample_code() {
	int nphase = 16;
	int ntap = 4;
	frac coef[16*4+1];
	short quant[16*4+1];
	int src_size = 720;
	int dst_size = 720/3;
	int centered = 1;
	frac cutoffscale = DOUBLE_TO_FRAC(1.0);
	int i, j;
	designfilterff(src_size, dst_size, nphase, ntap, centered, cutoffscale, 8, coef, quant);
	for (i=0; i<nphase; i++) {
		short c[8];
		extract_phase(c, quant, nphase, ntap, i);
		for (j=0; j<ntap; j++) {
			printf("%4d,", c[j]);
			// fill to register
		}
	}
}
#endif

#define VPOST_GMA_FILTER

#include <rpc_hld/ali_rpc_hld.h>

static INT32 gma_enhance_coef_default_bt709[] = {
	93, 314, 32, 0,
	-52, -173, 225, 0,
	225, -204, -21, 0, // csc matrix coef
};

#define MY_BITS	16

#define MY0	(93.484)
#define MY1	(314.486)
#define MY2	(31.7476)
#define MY3	(439.7176)

#define MC0_1 (51.542)
#define MC0_2 (224.8784)
#define MC1_1 (173.336)
#define MC1_2 (204.2796)
#define MC2_1 (224.8784)
#define MC2_2 (20.5989)
#define FLOOR(x) ((int)((x) + 0.5))

BOOL vpo_hal_generate_scale_coeff(
    short *pcoeff_table, UINT32 input_sample, UINT32 output_sample,
    UINT8 tap, UINT8 group_num, BOOL bDublicateCoeff
    )
{
    //UINT8 i;
    //short tmp0, tmp1, tmp2, tmp3;
    frac coeff_real[16*4+1];
    frac cutoffscale = DOUBLE_TO_FRAC(1.0);

    if (tap * group_num > 16*4)
        return FALSE;

    if(output_sample >= input_sample)//scaleup
    {
        cutoffscale = DOUBLE_TO_FRAC(0.9);
    }
    else//scaledown
    {
        cutoffscale = DOUBLE_TO_FRAC(0.9);
    }
    if(bDublicateCoeff)
    {
        cutoffscale = DOUBLE_TO_FRAC(1.0);
    }
    designfilterff(input_sample, output_sample, group_num, tap, 1, cutoffscale, 8, coeff_real, pcoeff_table);

    return TRUE;
}

void gma_m36f_enhance_coef_gen(int *coef_table, int brightness, int contrast
	, int saturation, int hue, int sharpness)
{
#if 0
    double Ly, Sy, Sc;
    double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9;
    frac sin_val = 0, cos_val = 0;
    libc_printf("%s : bright %d contrast %d saturation %d lue %d sharp %d\n", __FUNCTION__, brightness
    , contrast, saturation, hue, sharpness);
#endif

	int i = 0;	
//	if((brightness == 50) && (contrast == 50) && (saturation == 50) && (hue == 50))
	{
		for(i = 0;i < 12;i++)
			coef_table[i] = gma_enhance_coef_default_bt709[i];
	}
#if 0
	else
	{
		/* brightness, contrast, saturation value to double*/
		// Ly replace Lr, Lg, Lb : [-64, 64]
		Ly = ((double)brightness - 50) * 64 / 50; 

		// Sy replace Sr, Sg, Sr : [0, 2]
		if(contrast >= 50)
			Sy = 1 + ((double)contrast - 50) / 50;
		else
			//Sy = -2 + ((double)contrast) * 3 / 50;
			Sy = ((double)contrast)  / 50;

		// Sc : [0, 2]
		if(saturation >= 50)
			Sc = 1 + ((double)saturation - 50) / 50;
		else
			//Sc = -2 + ((double)saturation) * 3 / 50;
			Sc = ((double)saturation) / 50;

		Ly = Ly + (1 - Sy) * 64;
		
		/* sin and cos value for hue */
		if(hue == 50)
		{
			sin_val = 0;
			cos_val = FRAC_ONE;
		}
		else if((hue == 100) || (hue == 0))
		{
			sin_val = 0;
			cos_val = INT_TO_FRAC(-1);
		}
		else
		{
			int hue_in = 0;
			double hue_dou = (hue / 50) - 1;
			
			hue_in = DOUBLE_TO_FRAC(hue_dou);
			sin_val = sinffn(hue_in);
			cos_val = cosffn(hue_in);
		}


		tmp1 = Sc * MC0_1;
		tmp2 = Sc * MC0_2;
		tmp3 = Sc * MC1_1;
		tmp4 = Sc * MC1_2;
		tmp5 = Sc * MC2_1;
		tmp6 = Sc * MC2_2;
		
		/* generate coef table */
		coef_table[0] = FLOOR(Sy * MY0);
		coef_table[1] = FLOOR(Sy * MY1);
		coef_table[2] = FLOOR(Sy * MY2);
		coef_table[3] = FLOOR(Ly * MY3);

		tmp7 = -tmp1 * cos_val + tmp2 * sin_val;
		tmp8 = -tmp3 * cos_val - tmp4 * sin_val;
		tmp9 = tmp5 * cos_val - tmp6 * sin_val;
		
		coef_table[4] = FLOOR(tmp7 * Sy)>>FRAC_BITS;
		coef_table[5] = FLOOR(tmp8 * Sy)>>FRAC_BITS;
		coef_table[6] = FLOOR(tmp9 * Sy)>>FRAC_BITS;
		coef_table[7] = FLOOR((tmp7 + tmp8 + tmp9) * Ly)>>FRAC_BITS;

		tmp7 = tmp1 * sin_val + tmp2 * cos_val;
		tmp8 = tmp3 * sin_val - tmp4 * cos_val;
		tmp9 = -tmp5 * sin_val - tmp6 * cos_val;		
		coef_table[8] = FLOOR(tmp7 * Sy)>>FRAC_BITS;
		coef_table[9] = FLOOR(tmp8 * Sy)>>FRAC_BITS;
		coef_table[10] = FLOOR(tmp9 * Sy)>>FRAC_BITS;
		coef_table[11] = FLOOR((tmp7 + tmp8 + tmp9) * Ly)>>FRAC_BITS;
	}
#endif	
	/* sharpness value */
	if(sharpness == 50)
		sharpness = 0;
	else
		sharpness = (sharpness - 50) / 10;
	
	coef_table[12] = sharpness;
    
}

//#endif // defined(DUAL_ENABLE) && defined(MAIN_CPU)
#endif

