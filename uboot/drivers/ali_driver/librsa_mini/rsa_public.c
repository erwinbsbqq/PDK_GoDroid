#include "NN.h"
#include "lib_rsa.h"

#if 1
/* Computes a = b + c. Returns carry.

	 Lengths: a[digits], b[digits], c[digits].
 */
static UINT32 ALI_Add (a, b, c, digits)
UINT32 *a, *b, *c;
unsigned int digits;
{
	UINT32 temp, carry = 0;

	if(digits)
		do {
			if((temp = (*b++) + carry) < carry)
				temp = *c++;
			else
				if((temp += *c) < *c++)
					carry = 1;
				else
					carry = 0;
			*a++ = temp;
		}while(--digits);

	return (carry);
}

/* Assigns a = b. */

static void ALI_Assign (a, b, digits)
UINT32 *a, *b;
unsigned int digits;
{
	if(digits) {
		do {
			*a++ = *b++;
		}while(--digits);
	}
}

/* Returns the significant length of a in digits. */

static unsigned int ALI_Digits (a, digits)
UINT32 *a;
unsigned int digits;
{

	if(digits) {
		digits--;

		do {
			if(*(a+digits))
				break;
		}while(digits--);

		return(digits + 1);
	}

	return(digits);
}



/* Computes a = b * 2^c (i.e., shifts left c bits), returning carry.

	 Requires c < ALI_DIGIT_BITS. */

static UINT32 ALI_LShift (a, b, c, digits)
UINT32 *a, *b;
unsigned int c, digits;
{
	UINT32 temp, carry = 0;
	unsigned int t;

	if(c < ALI_DIGIT_BITS)
		if(digits) {

			t = ALI_DIGIT_BITS - c;

			do {
				temp = *b++;
				*a++ = (temp << c) | carry;
				carry = c ? (temp >> t) : 0;
			}while(--digits);
		}

	return (carry);
}

/* Computes a = c div 2^c (i.e., shifts right c bits), returning carry.

	 Requires: c < ALI_DIGIT_BITS. */

static UINT32 ALI_RShift (a, b, c, digits)
UINT32 *a, *b;
unsigned int c, digits;
{
	UINT32 temp, carry = 0;
	unsigned int t;

	if(c < ALI_DIGIT_BITS)
		if(digits) {

			t = ALI_DIGIT_BITS - c;

			do {
				digits--;
				temp = *(b+digits);
				*(a+digits) = (temp >> c) | carry;
				carry = c ? (temp << t) : 0;
			}while(digits);
		}

	return (carry);
}

/* Computes a = b - c. Returns borrow.

	 Lengths: a[digits], b[digits], c[digits].
 */
static UINT32 ALI_Sub (a, b, c, digits)
UINT32 *a, *b, *c;
unsigned int digits;
{
	UINT32 temp, borrow = 0;

	if(digits)
		do {
			if((temp = (*b++) - borrow) == MAX_ALI_DIGIT)
				temp = MAX_ALI_DIGIT - *c++;
			else
				if((temp -= *c) > (MAX_ALI_DIGIT - *c++))
					borrow = 1;
				else
					borrow = 0;
			*a++ = temp;
		}while(--digits);

	return(borrow);
}

/* Computes a * b, result stored in high and low. */
 
static void dmult( a, b, high, low)
UINT32          a, b;
UINT32         *high;
UINT32         *low;
{
	UINT16 al, ah, bl, bh;
	UINT32 m1, m2, m, ml, mh, carry = 0;

	al = (UINT16)LOW_HALF(a);
	ah = (UINT16)HIGH_HALF(a);
	bl = (UINT16)LOW_HALF(b);
	bh = (UINT16)HIGH_HALF(b);

	*low = (UINT32) al*bl;
	*high = (UINT32) ah*bh;

	m1 = (UINT32) al*bh;
	m2 = (UINT32) ah*bl;
	m = m1 + m2;

	if(m < m1)
		carry = 1 << (ALI_DIGIT_BITS / 2);

	ml = (m & MAX_ALI_HALF_DIGIT) << (ALI_DIGIT_BITS / 2);
	mh = m >> (ALI_DIGIT_BITS / 2);

	*low += ml;

	if(*low < ml)
		carry++;

	*high += carry + mh;
}


static UINT32 subdigitmult(a, b, c, d, digits)
UINT32 *a, *b, c, *d;
unsigned int digits;
{
	UINT32 borrow, thigh, tlow;
	unsigned int i;

	borrow = 0;

	if(c != 0) {
		for(i = 0; i < digits; i++) {
			dmult(c, d[i], &thigh, &tlow);
			if((a[i] = b[i] - borrow) > (MAX_ALI_DIGIT - borrow))
				borrow = 1;
			else
				borrow = 0;
			if((a[i] -= tlow) > (MAX_ALI_DIGIT - tlow))
				borrow++;
			borrow += thigh;
		}
	}

	return (borrow);
}




static void R_memset(output, value, len)
UINT8 *output;                 /* output block */
int value;                      /* value */
unsigned int len;               /* length of block */
{
	if(len != 0) {
		do {
			*output++ = (unsigned char)value;
		}while(--len != 0);
	}
}



/* Assigns a = 0. */

static void ALI_AssignZero (a, digits)
UINT32 *a;
unsigned int digits;
{
	if(digits) {
		do {
			*a++ = 0;
		}while(--digits);
	}
}


/* Returns the significant length of a in bits, where a is a digit. */

static unsigned int ALI_DigitBits (a)
UINT32 a;
{
	unsigned int i;

	for (i = 0; i < ALI_DIGIT_BITS; i++, a >>= 1)
		if (a == 0)
			break;

	return (i);
}
/* Encodes b into character string a, where character string is ordered
   from most to least significant.

	 Lengths: a[len], b[digits].
	 Assumes ALI_Bits (b, digits) <= 8 * len. (Otherwise most significant
	 digits are truncated.)
 */

/* Returns sign of a - b. */

static int ALI_Cmp (a, b, digits)
UINT32 *a, *b;
unsigned int digits;
{

	if(digits) {
		do {
			digits--;
			if(*(a+digits) > *(b+digits))
				return(1);
			if(*(a+digits) < *(b+digits))
				return(-1);
		}while(digits);
	}

	return (0);
}

/* Computes a = c div d and b = c mod d.

	 Lengths: a[cDigits], b[dDigits], c[cDigits], d[dDigits].
	 Assumes d > 0, cDigits < 2 * MAX_ALI_DIGITS,
					 dDigits < MAX_ALI_DIGITS.
*/

static void ALI_Div (a, b, c, cDigits, d, dDigits)
UINT32 *a, *b, *c, *d;
unsigned int cDigits, dDigits;
{
	UINT32 ai, cc[2*MAX_ALI_DIGITS+1], dd[MAX_ALI_DIGITS], s;
	UINT32 t[2], u, v, *ccptr;
	UINT16 aHigh, aLow, cHigh, cLow;
	int i;
	unsigned int ddDigits, shift;

	ddDigits = ALI_Digits (d, dDigits);
	if(ddDigits == 0)
		return;

	shift = ALI_DIGIT_BITS - ALI_DigitBits (d[ddDigits-1]);
	ALI_AssignZero (cc, ddDigits);
	cc[cDigits] = ALI_LShift (cc, c, shift, cDigits);
	ALI_LShift (dd, d, shift, ddDigits);
	s = dd[ddDigits-1];

	ALI_AssignZero (a, cDigits);

	for (i = cDigits-ddDigits; i >= 0; i--) {
		if (s == MAX_ALI_DIGIT)
			ai = cc[i+ddDigits];
		else {
			ccptr = &cc[i+ddDigits-1];

			s++;
			cHigh = (UINT16)HIGH_HALF (s);
			cLow = (UINT16)LOW_HALF (s);

			*t = *ccptr;
			*(t+1) = *(ccptr+1);

			if (cHigh == MAX_ALI_HALF_DIGIT)
				aHigh = (UINT16)HIGH_HALF (*(t+1));
			else
				aHigh = (UINT16)(*(t+1) / (cHigh + 1));
			u = (UINT32)aHigh * (UINT32)cLow;
			v = (UINT32)aHigh * (UINT32)cHigh;
			if ((*t -= TO_HIGH_HALF (u)) > (MAX_ALI_DIGIT - TO_HIGH_HALF (u)))
				t[1]--;
			*(t+1) -= HIGH_HALF (u);
			*(t+1) -= v;

			while ((*(t+1) > cHigh) ||
						 ((*(t+1) == cHigh) && (*t >= TO_HIGH_HALF (cLow)))) {
				if ((*t -= TO_HIGH_HALF (cLow)) > MAX_ALI_DIGIT - TO_HIGH_HALF (cLow))
					t[1]--;
				*(t+1) -= cHigh;
				aHigh++;
			}

			if (cHigh == MAX_ALI_HALF_DIGIT)
				aLow = (UINT16)LOW_HALF (*(t+1));
			else
				aLow =
			(UINT16)((TO_HIGH_HALF (*(t+1)) + HIGH_HALF (*t)) / (cHigh + 1));
			u = (UINT32)aLow * (UINT32)cLow;
			v = (UINT32)aLow * (UINT32)cHigh;
			if ((*t -= u) > (MAX_ALI_DIGIT - u))
				t[1]--;
			if ((*t -= TO_HIGH_HALF (v)) > (MAX_ALI_DIGIT - TO_HIGH_HALF (v)))
				t[1]--;
			*(t+1) -= HIGH_HALF (v);

			while ((*(t+1) > 0) || ((*(t+1) == 0) && *t >= s)) {
				if ((*t -= s) > (MAX_ALI_DIGIT - s))
					t[1]--;
				aLow++;
			}

			ai = TO_HIGH_HALF (aHigh) + aLow;
			s--;
		}

		cc[i+ddDigits] -= subdigitmult(&cc[i], &cc[i], ai, dd, ddDigits);

		while (cc[i+ddDigits] || (ALI_Cmp (&cc[i], dd, ddDigits) >= 0)) {
			ai++;
			cc[i+ddDigits] -= ALI_Sub (&cc[i], &cc[i], dd, ddDigits);
		}

		a[i] = ai;
	}

	ALI_AssignZero (b, dDigits);
	ALI_RShift (b, cc, shift, ddDigits);

	R_memset ((UINT8 *)cc, 0, sizeof (cc));
	R_memset ((UINT8 *)dd, 0, sizeof (dd));
}


/* Computes a = b mod c.

	 Lengths: a[cDigits], b[bDigits], c[cDigits].
	 Assumes c > 0, bDigits < 2 * MAX_ALI_DIGITS, cDigits < MAX_ALI_DIGITS.
*/
static void ALI_Mod (a, b, bDigits, c, cDigits)
UINT32 *a, *b, *c;
unsigned int bDigits, cDigits;
{
  UINT32 t[2 * MAX_ALI_DIGITS];
  
	ALI_Div (t, a, b, bDigits, c, cDigits);
  
  /* Zeroize potentially sensitive information.
	 */
  R_memset ((UINT8 *)t, 0, sizeof (t));
}

/* Computes a = b * c.

	 Lengths: a[2*digits], b[digits], c[digits].
	 Assumes digits < MAX_ALI_DIGITS.
*/

static void ALI_Mult (a, b, c, digits)
UINT32 *a, *b, *c;
unsigned int digits;
{
	UINT32 t[2*MAX_ALI_DIGITS];
	UINT32 dhigh, dlow, carry;
	unsigned int bDigits, cDigits, i, j;

	ALI_AssignZero (t, 2 * digits);

	bDigits = ALI_Digits (b, digits);
	cDigits = ALI_Digits (c, digits);

	for (i = 0; i < bDigits; i++) {
		carry = 0;
		if(*(b+i) != 0) {
			for(j = 0; j < cDigits; j++) {
				dmult(*(b+i), *(c+j), &dhigh, &dlow);
				if((*(t+(i+j)) = *(t+(i+j)) + carry) < carry)
					carry = 1;
				else
					carry = 0;
				if((*(t+(i+j)) += dlow) < dlow)
					carry++;
				carry += dhigh;
			}
		}
		*(t+(i+cDigits)) += carry;
	}


	ALI_Assign(a, t, 2 * digits);

	/* Clear sensitive information. */

	R_memset((UINT8 *)t, 0, sizeof (t));
}


/* Computes a = b * c mod d.

   Lengths: a[digits], b[digits], c[digits], d[digits].
   Assumes d > 0, digits < MAX_ALI_DIGITS.
 */
static void ALI_ModMult (a, b, c, d, digits)
UINT32 *a, *b, *c, *d;
unsigned int digits;
{
	UINT32 t[2*MAX_ALI_DIGITS];

	ALI_Mult (t, b, c, digits);
  ALI_Mod (a, t, 2 * digits, d, digits);

	/* Zeroize potentially sensitive information.
	 */
  R_memset ((UINT8 *)t, 0, sizeof (t));
}


/* Decodes character string b into a, where character string is ordered
	 from most to least significant.

	 Lengths: a[digits], b[len].
	 Assumes b[i] = 0 for i < len - digits * ALI_DIGIT_LEN. (Otherwise most
	 significant bytes are truncated.)
 */
static void ALI_Decode (a, digits, b, len)
UINT32 *a;
unsigned char *b;
unsigned int digits, len;
{
  UINT32 t;
  int j;
  unsigned int i, u;
  
  for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
    t = 0;
    for (u = 0; j >= 0 && u < ALI_DIGIT_BITS; j--, u += 8)
			t |= ((UINT32)b[j]) << u;
		a[i] = t;
  }
  
  for (; i < digits; i++)
    a[i] = 0;
}


/* Computes a = b^c mod d.

   Lengths: a[dDigits], b[dDigits], c[cDigits], d[dDigits].
	 Assumes d > 0, cDigits > 0, dDigits < MAX_ALI_DIGITS.
 */
static void ALI_ModExp (a, b, c, cDigits, d, dDigits)
UINT32 *a, *b, *c, *d;
unsigned int cDigits, dDigits;
{
    UINT32 bPower[3][MAX_ALI_DIGITS], ci, t[MAX_ALI_DIGITS];
    int i;
    unsigned int ciBits, j, s;

    /* Store b, b^2 mod d, and b^3 mod d.
    */
    ALI_Assign (bPower[0], b, dDigits);
    ALI_ModMult (bPower[1], bPower[0], b, d, dDigits);
    ALI_ModMult (bPower[2], bPower[1], b, d, dDigits);

    ALI_AssignZero(t,dDigits);
    t[0] = 1;

    cDigits = ALI_Digits (c, cDigits);
    for (i = cDigits - 1; i >= 0; i--) {
    ci = c[i];
    ciBits = ALI_DIGIT_BITS;

    /* Scan past leading zero bits of most significant digit.
     */
    if (i == (int)(cDigits - 1)) {
    	while (! DIGIT_2MSB (ci)) {
    		ci <<= 2;
    		ciBits -= 2;
    	}
    }

    for (j = 0; j < ciBits; j += 2, ci <<= 2) {
    /* Compute t = t^4 * b^s mod d, where s = two MSB's of ci.
    	 */
    ALI_ModMult (t, t, t, d, dDigits);
    ALI_ModMult (t, t, t, d, dDigits);
    if ((s = DIGIT_2MSB (ci)) != 0)
    ALI_ModMult (t, t, bPower[s-1], d, dDigits);
    }
    }

    ALI_Assign (a, t, dDigits);

    /* Zeroize potentially sensitive information.
    */
    R_memset ((UINT8 *)bPower, 0, sizeof (bPower));
    R_memset ((UINT8 *)t, 0, sizeof (t));
}
#endif

static void ALI_Encode (a, len, b, digits)
UINT32 *b;
unsigned char *a;
unsigned int digits, len;
{
	UINT32 t;
	int j;
	unsigned int i, u;

	for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
		t = b[i];
		for (u = 0; j >= 0 && u < ALI_DIGIT_BITS; j--, u += 8)
			a[j] = (unsigned char)(t >> u);
	}

	for (; j >= 0; j--)
		a[j] = 0;
}
/* Raw RSA public-key operation. Output has same length as modulus.

	 Requires input < modulus.
*/
int ali_rsapublicfunc(output, outputLen, input, inputLen, publicKey)
unsigned char *output;          /* output block */
unsigned int *outputLen;        /* length of output block */
unsigned char *input;           /* input block */
unsigned int inputLen;          /* length of input block */
R_RSA_PUBLIC_KEY *publicKey;    /* RSA public key */
{
	UINT32 c[MAX_ALI_DIGITS], e[MAX_ALI_DIGITS], m[MAX_ALI_DIGITS],
		n[MAX_ALI_DIGITS];
	unsigned int eDigits, nDigits,a;


		/* decode the required RSA function input data */
	ALI_Decode(m, MAX_ALI_DIGITS, input, inputLen);
	ALI_Decode(n, MAX_ALI_DIGITS, publicKey->modulus, ALI_RSA_MODULUS_LEN);
	ALI_Decode(e, MAX_ALI_DIGITS, publicKey->exponent, ALI_RSA_MODULUS_LEN);
	nDigits = ALI_Digits(n, MAX_ALI_DIGITS);
	eDigits = ALI_Digits(e, MAX_ALI_DIGITS);
	if(ALI_Cmp(m, n, nDigits) >= 0)
		return RET_FAILURE;
	*outputLen = (publicKey->bits + 7) / 8;

	/* Compute c = m^e mod n.  To perform actual RSA calc.*/
	

	ALI_ModExp (c, m, e, eDigits, n, nDigits);

	/* encode output to standard form */
	ALI_Encode (output, *outputLen, c, nDigits);
	/* Clear sensitive information. */

	R_memset((UINT8 *)c, 0, sizeof(c));
	R_memset((UINT8 *)m, 0, sizeof(m));

	return RET_SUCCESS;
}

/*
 *Add the padding for signature, the padding format means the algorithm of SHA is SHA256 
 */
static UINT32 __aliasix_sha_result_conversion(UINT8 *sha_padding, UINT8 *result)
{
    UINT32 i = 0;
    static unsigned char DIGEST_INFO_A[] =
    {
    	0x00,0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,
    };
    static unsigned char DIGEST_INFO_B[] =
    {
        0x05,0x00,0x04,0x20
    };
	sha_padding[0]=0;
	sha_padding[1]=0x1;
    for(i=2;i<204;i++)
		sha_padding[i]=0xff;
	memcpy(&sha_padding[i],DIGEST_INFO_A,sizeof(DIGEST_INFO_A));
	i+=sizeof(DIGEST_INFO_A);
	sha_padding[i++]=0x1;
	memcpy(&sha_padding[i],DIGEST_INFO_B,sizeof(DIGEST_INFO_B));
	i+=sizeof(DIGEST_INFO_B);
	memcpy(&sha_padding[i],result,ALIASIX_HASH_DIGEST_LEN);
    return 0;
}
/*
 * Verify the given data
 */
RET_CODE ali_rsa_result_compare(UINT8 *rsa_result, void *sha_result)
{
    UINT32 i = 0;
    UINT8  sha_padding[ALIASIX_RSA_COMPARE_LEN];
    __aliasix_sha_result_conversion(sha_padding,sha_result);
    
    if(0 != memcmp((void *)rsa_result, 
                    (void *)sha_padding, 
                    ALIASIX_RSA_COMPARE_LEN))
    {
            return RET_FAILURE;
    }
        /*Redundant check*/
    for(i=0;i<ALIASIX_RSA_COMPARE_LEN;i+=4)
    {
        if (*(volatile UINT32*)((UINT32)rsa_result+i) !=
                *(volatile UINT32 *)((UINT32)sha_padding+i))
        {     
            return RET_FAILURE;
        }
    }  
    return RET_SUCCESS;
}

