/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be 
     disclosed to unauthorized individual.    
*    File: rsa_public.c
*   
*    Description: 
*       
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#include <ali/basic_types.h>
#include <ali/retcode.h>
#include "rsa_public.h"

static UINT32 bpower[3][MAX_ALI_DIGITS]; 
/* Computes a = b + c. Returns carry.

     Lengths: a[digits], b[digits], c[digits].
 */
static UINT32 ali_add (a, b, c, digits)
UINT32 *a, *b, *c;
UINT32 digits;
{
    UINT32 temp=0, 
           carry = 0;
    
    if((NULL == a)||(NULL==b)||(NULL==c)||(0==digits))
    {
        return carry;
    }
    if(digits)
    {
        do 
        {
            if((temp = (*b++) + carry) < carry)
            {
                temp = *c++;
            }
            else
            {
                if((temp += *c) < *c++)
                {
                    carry = 1;
                }
                else
                {
                    carry = 0;
                }
            }
            *a++ = temp;
        }while(--digits);
    }
    return (carry);
}

/* Assigns a = b. */

static void ali_assign (a, b, digits)
UINT32 *a, *b;
UINT32 digits;
{
    if((NULL==a)||(NULL==b)||(0==digits))
    {
        return;    
    }
    if(digits) 
    {
        do 
        {
            *a++ = *b++;
        }while(--digits);
    }
}

/* Returns the significant length of a in digits. */

static UINT32 ali_digits (a, digits)
UINT32 *a;
UINT32 digits;
{
    if((NULL==a)||(0==digits))
    {
        return digits;
    }
    if(digits) 
    {
        do 
        {
            digits-- ;
            if(*(a+digits))
            {
                break;
            }
        }while(digits);

        return(digits + 1);
    }

    return(digits);
}

/* Returns sign of a - b. */

static INT32 ali_cmp (a, b, digits)
UINT32 *a, *b;
UINT32 digits;
{
    if((NULL==a)||(NULL==b)||(0==digits))
    {
        return 0;
    }
    if(digits) 
    {
        do 
        {
            digits--;
            if(*(a+digits) > *(b+digits))
            {
                return(1);
            }
            if(*(a+digits) < *(b+digits))
            {
                return(-1);
            }
        }while(digits);
    }

    return (0);
}

/* Computes a = b * 2^c (i.e., shifts left c bits), returning carry.

     Requires c < ALI_DIGIT_BITS. */

static UINT32 ali_lshift (a, b, c, digits)
UINT32 *a, *b;
UINT32 c, digits;
{
    UINT32 temp = 0, 
           carry = 0;
    UINT32 t = 0;
    
    if((NULL==a)||(NULL==b)||(0==digits))
    {
        return carry;
    }
    if(c < ALI_DIGIT_BITS)
    {
        if(digits) 
        {

            t = ALI_DIGIT_BITS - c;

            do 
            {
                temp = *b++;
                *a++ = (temp << c) | carry;
                carry = c ? (temp >> t) : 0;
            }while(--digits);
        }
    }

    return (carry);
}

/* Computes a = c div 2^c (i.e., shifts right c bits), returning carry.

     Requires: c < ALI_DIGIT_BITS. */

static UINT32 ali_rshift (a, b, c, digits)
UINT32 *a, *b;
UINT32 c, digits;
{
    UINT32 temp = 0 , 
           carry = 0;
    UINT32 t = 0;

    if((NULL==a)||(NULL==b)||(0==digits))
    {
        return carry;
    }
    if(c < ALI_DIGIT_BITS)
    {
        if(digits) 
        {

            t = ALI_DIGIT_BITS - c;

            do 
            {
                digits--;
                temp = *(b+digits);
                *(a+digits) = (temp >> c) | carry;
                carry = c ? (temp << t) : 0;
            }while(digits);
        }
    }

    return (carry);
}

/* Computes a = b - c. Returns borrow.

     Lengths: a[digits], b[digits], c[digits].
 */
static UINT32 ali_sub (a, b, c, digits)
UINT32 *a, *b, *c;
UINT32 digits;
{
    UINT32 temp = 0 ,
           borrow = 0;
    
    if((NULL==a)||(NULL==b)||(NULL==c)||(0==digits))
    {
        return borrow;
    }
    if(digits)
    {
        do 
        {
            if(MAX_ALI_DIGIT == (temp = (*b++) - borrow))
            {
                temp = MAX_ALI_DIGIT - *c++;
            }
            else
            {
                if((temp -= *c) > (MAX_ALI_DIGIT - *c++))
                {
                    borrow = 1;
                }
                else
                {
                    borrow = 0;
                }
            }
            *a++ = temp;
        }while(--digits);
    }
    return(borrow);
}

/* Computes a * b, result stored in high and low. */
 
static void dmult( a, b, high, low)
UINT32          a, b;
UINT32         *high;
UINT32         *low;
{
    UINT16 al = 0,
           ah = 0, 
           bl = 0, 
           bh = 0;
    UINT32 m1 = 0, 
           m2 = 0 , 
           m = 0, 
           ml = 0, 
           mh = 0, 
           carry = 0;
    
    if((NULL == low)||(NULL == high))
    {
        return;
    }
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
    {
        carry = 1 << (ALI_DIGIT_BITS / 2);
    }

    ml = (m & MAX_ALI_HALF_DIGIT) << (ALI_DIGIT_BITS / 2);
    mh = m >> (ALI_DIGIT_BITS / 2);

    *low += ml;

    if(*low < ml)
    {
        carry++;
    }

    *high += carry + mh;
}


static UINT32 subdigitmult(a, b, c, d, digits)
UINT32 *a, *b, c, *d;
UINT32 digits;
{
    UINT32 borrow =0, 
           thigh =0, 
           tlow =0;
    UINT32 i = 0;

    borrow = 0;
    if((NULL==a)||(NULL==b)||(NULL==d)||(0==digits))
    {
        return borrow;
    }
    if(c != 0) 
    {
        for(i = 0; i < digits; i++) 
        {
            dmult(c, d[i], &thigh, &tlow);
            if((a[i] = b[i] - borrow) > (MAX_ALI_DIGIT - borrow))
            {
                borrow = 1;
            }
            else
            {
                borrow = 0;
            }
            if((a[i] -= tlow) > (MAX_ALI_DIGIT - tlow))
            {
                borrow++;
            }
            borrow += thigh;
        }
    }

    return (borrow);
}

static void r_memset(output, value, len)
UINT8 *output;                 /* output block */
INT32 value;                      /* value */
UINT32 len;               /* length of block */
{
    if(NULL == output)
    {
        return;
    }
    if(len != 0) 
    {
        do 
        {
            *output++ = (UINT8)value;
        }while(--len != 0);
    }
}



/* Assigns a = 0. */

static void ali_assign_zero (a, digits)
UINT32 *a;
UINT32 digits;
{
    if(NULL==a)
    {
        return;    
    }
    if(digits) 
    {
        do 
        {
            *a++ = 0;
        }while(--digits);
    }
}


/* Returns the significant length of a in bits, where a is a digit. */

static UINT32 ali_digit_bits (a)
UINT32 a;
{
    UINT32 i = 0;

    for (i = 0; i < ALI_DIGIT_BITS; i++, a >>= 1)
    {
        if (0 == a)
        {
            break;
        }
    }

    return (i);
}


/* Computes a = c div d and b = c mod d.

     Lengths: a[cdigits], b[ddigits], c[cdigits], d[ddigits].
     Assumes d > 0, cdigits < 2 * MAX_ALI_DIGITS,
                     ddigits < MAX_ALI_DIGITS.
*/

static RET_CODE ali_div (a, b, c, cdigits, d, ddigits)
UINT32 *a, *b, *c, *d;
UINT32 cdigits,
       ddigits;
{
    UINT32 ai =0,
           s =0;
    UINT32 *cc = NULL, 
           *dd = NULL;            
    UINT32 t[2], 
           u =0, 
           v =0, 
           *ccptr = NULL;
    RET_CODE  ret = RET_FAILURE;
    UINT16 ahigh =0, 
           alow =0, 
           chigh =0, 
           clow = 0;
    INT32 i = 0;
    UINT32 dddigits =0, 
                 shift =0;

    if((NULL==a)||(NULL == b)||(NULL == c)||(NULL == d))
    {
        return ret;
    }
    cc = (UINT32 *)malloc((2*MAX_ALI_DIGITS+1)*4);
    if(NULL == cc)
    {
        return RET_FAILURE ;
    }

    dd = (UINT32 *)malloc(MAX_ALI_DIGITS*4);
    if(NULL == dd)
    {
        free(cc);
        return RET_FAILURE ;
    }


    dddigits = ali_digits (d, ddigits);
    if(0 == dddigits)
    {
        free(cc);
        free(dd);
        return RET_FAILURE;
    }

    shift = ALI_DIGIT_BITS - ali_digit_bits (d[dddigits-1]);
    ali_assign_zero (cc, dddigits);
    cc[cdigits] = ali_lshift (cc, c, shift, cdigits);
    ret = ali_lshift (dd, d, shift, dddigits);
    s = dd[dddigits-1];

    ali_assign_zero (a, cdigits);

    for (i = cdigits-dddigits; i >= 0; i--) 
    {
        if (MAX_ALI_DIGIT == s)
        {
            ai = cc[i+dddigits];
        }
        else 
        {
            ccptr = &cc[i+dddigits-1];

            s++;
            chigh = (UINT16)HIGH_HALF (s);
            clow = (UINT16)LOW_HALF (s);

            *t = *ccptr;
            *(t+1) = *(ccptr+1);

            if (MAX_ALI_HALF_DIGIT == chigh)
            {
                ahigh = (UINT16)HIGH_HALF (*(t+1));
            }
            else
            {
                ahigh = (UINT16)(*(t+1) / (chigh + 1));
            }
            u = (UINT32)ahigh * (UINT32)clow;
            v = (UINT32)ahigh * (UINT32)chigh;
            if ((*t -= TO_HIGH_HALF (u)) > (MAX_ALI_DIGIT - TO_HIGH_HALF (u)))
            {
                t[1]--;
            }
            *(t+1) -= HIGH_HALF (u);
            *(t+1) -= v;

            while ((*(t+1) > chigh) ||
                         ((*(t+1) == chigh) && (*t >= TO_HIGH_HALF (clow)))) 
            {
                if ((*t -= TO_HIGH_HALF (clow)) > \
                       (MAX_ALI_DIGIT - TO_HIGH_HALF (clow)))
                {
                    t[1]--;
                }
                *(t+1) -= chigh;
                ahigh++;
            }

            if (MAX_ALI_HALF_DIGIT == chigh)
            {
                alow = (UINT16)LOW_HALF (*(t+1));
            }
            else
            {
                alow = (UINT16)((TO_HIGH_HALF (*(t+1)) \
                           +HIGH_HALF (*t)) / (chigh + 1));
            }
            u = (UINT32)alow * (UINT32)clow;
            v = (UINT32)alow * (UINT32)chigh;
            if ((*t -= u) > (MAX_ALI_DIGIT - u))
            {
                t[1]--;
            }
            if ((*t -= TO_HIGH_HALF (v)) > \
                    (MAX_ALI_DIGIT - TO_HIGH_HALF (v)))
            {
                t[1]--;
            }
            *(t+1) -= HIGH_HALF (v);

            while ((*(t+1) > 0) || ((0 == *(t+1)) && *t >= s)) 
            {
                if ((*t -= s) > (MAX_ALI_DIGIT - s))
                {
                    t[1]--;
                }
                alow++;
            }

            ai = TO_HIGH_HALF (ahigh) + alow;
            s--;
        }

        cc[i+dddigits] -= subdigitmult(&cc[i], &cc[i], ai, dd, dddigits);

        while (cc[i+dddigits] || (ali_cmp (&cc[i], dd, dddigits) >= 0)) 
        {
            ai++;
            cc[i+dddigits] -= ali_sub (&cc[i], &cc[i], dd, dddigits);
        }

        a[i] = ai;
    }

    ali_assign_zero (b, ddigits);
    ret =ali_rshift (b, cc, shift, dddigits);

    r_memset ((UINT8 *)cc, 0, (2*MAX_ALI_DIGITS+1)*4);
    r_memset ((UINT8 *)dd, 0, MAX_ALI_DIGITS*4);
	
	free(cc);
	free(dd);
    return RET_SUCCESS ;
}


/* Computes a = b mod c.

     Lengths: a[cdigits], b[bdigits], c[cdigits].
     Assumes c > 0, bdigits < 2 * MAX_ALI_DIGITS, cdigits < MAX_ALI_DIGITS.
*/
static RET_CODE ali_mod (a, b, bdigits, c, cdigits)
UINT32 *a, *b, *c;
UINT32 bdigits, cdigits;
{
  UINT32 *t = NULL;
  RET_CODE ret = RET_FAILURE ;

  if((NULL == a)||(NULL == b)||(NULL==c))
  {
      return ret;
  }
  t = (UINT32 *)malloc(2 * MAX_ALI_DIGITS*4);
  if(NULL == t )
  {
    return RET_FAILURE;
  }
  ret = ali_div (t, a, b, bdigits, c, cdigits);
  if( ret != RET_SUCCESS )
  {
    free(t);
    return ret ;
  }
  
  /* Zeroize potentially sensitive information.
     */
  r_memset ((UINT8 *)t, 0, 2 * MAX_ALI_DIGITS*4);
  free(t);
  return RET_SUCCESS ;
}

/* Computes a = b * c.

     Lengths: a[2*digits], b[digits], c[digits].
     Assumes digits < MAX_ALI_DIGITS.
*/

static RET_CODE ali_mult (a, b, c, digits)
UINT32 *a, 
       *b, 
       *c;
UINT32 digits;
{
    UINT32 *t = NULL;
    UINT32 dhigh = 0, 
           dlow = 0, 
           carry = 0;
    UINT32 bdigits =0, 
                 cdigits =0 , 
             i = 0, 
             j = 0;

    if((NULL == a)||(NULL == b)||(NULL == c))
    {
        return RET_FAILURE;
    }
    t = (UINT32 *)malloc(2*MAX_ALI_DIGITS*4);
    if(NULL == t)
    {
       return RET_FAILURE;
    }

    ali_assign_zero(t, 2 * digits);

    bdigits = ali_digits (b, digits);
    cdigits = ali_digits (c, digits);

    for (i = 0; i < bdigits; i++) 
    {
        carry = 0;
        if(*(b+i) != 0) 
        {
            for(j = 0; j < cdigits; j++) 
            {
                dmult(*(b+i), *(c+j), &dhigh, &dlow);
                if((*(t+(i+j)) = *(t+(i+j)) + carry) < carry)
                {
                    carry = 1;
                }
                else
                {
                    carry = 0;
                }
                if((*(t+(i+j)) += dlow) < dlow)
                {
                    carry++;
                }
                carry += dhigh;
            }
        }
        *(t+(i+cdigits)) += carry;
    }


    ali_assign(a, t, 2 * digits);

    /* Clear sensitive information. */

    r_memset((UINT8 *)t, 0, 2*MAX_ALI_DIGITS*4);
    free(t);
	return RET_SUCCESS ;
}


/* Computes a = b * c mod d.

   Lengths: a[digits], b[digits], c[digits], d[digits].
   Assumes d > 0, digits < MAX_ALI_DIGITS.
 */
static RET_CODE ali_mod_mult (a, b, c, d, digits)
UINT32 *a, *b, *c, *d;
UINT32 digits;
{
    UINT32 *t = NULL;
    RET_CODE ret = RET_FAILURE ;
    
    if((NULL == a)||(NULL == b)||(NULL == c)||(NULL == d))
    {
        return RET_FAILURE;
    }
    t= (UINT32 *)malloc(2*MAX_ALI_DIGITS*4);
    if(NULL == t )
    {
        return RET_FAILURE;
    }

    ret = ali_mult (t, b, c, digits);
    if(ret != RET_SUCCESS )
    {
        free(t);
        return ret ;
    }
    ret = ali_mod (a, t, 2 * digits, d, digits);
    if(ret != RET_SUCCESS )
    {
        free(t);
        return ret ;
    }

    /* Zeroize potentially sensitive information.
     */
    r_memset ((UINT8 *)t, 0, 2*MAX_ALI_DIGITS*4);
    free(t);
  	return RET_SUCCESS ;
}


/* Decodes character string b into a, where character string is ordered
     from most to least significant.

     Lengths: a[digits], b[len].
     Assumes b[i] = 0 for i < len - digits * ALI_DIGIT_LEN. (Otherwise most
     significant bytes are truncated.)
 */
static void ali_decode (a, digits, b, len)
UINT32 *a;
UINT8 *b;
UINT32 digits, 
             len;
{
  UINT32 t =0;
  INT32 j =0;
  UINT32 i =0, 
         u = 0;
  
    if((NULL==a)||(NULL==b))
    {
        return;
    }
 
  for (i = 0, j = len - 1; i < digits && j >= 0; i++) 
  {
    t = 0;
    for (u = 0; j >= 0 && u < ALI_DIGIT_BITS; j--, u += 8)
    {
        t |= ((UINT32)b[j]) << u;
    }
        a[i] = t;
  }
  
  for (; i < digits; i++)
  {
    a[i] = 0;
  }
}

/* Encodes b into character string a, where character string is ordered
   from most to least significant.

     Lengths: a[len], b[digits].
     Assumes ALI_Bits (b, digits) <= 8 * len. (Otherwise most significant
     digits are truncated.)
 */
static void ali_encode (a, len, b, digits)
UINT32 *b;
UINT8 *a;
UINT32 digits, 
             len;
{
    UINT32 t =0;
    INT32 j = 0;
    UINT32 i =0, 
                 u = 0;

    if((NULL == a)||(NULL == b))
    {
        return;
    }
    for (i = 0, j = len - 1; i < digits && j >= 0; i++) 
    {
        t = b[i];
        for (u = 0; j >= 0 && u < ALI_DIGIT_BITS; j--, u += 8)
        {
            a[j] = (UINT8)(t >> u);
        }
    }

    for (; j >= 0; j--)
    {
        a[j] = 0;
    }
}
/* Computes a = b^c mod d.

   Lengths: a[ddigits], b[ddigits], c[cdigits], d[ddigits].
     Assumes d > 0, cdigits > 0, ddigits < MAX_ALI_DIGITS.
 */

static RET_CODE ali_mod_exp (a, b, c, cdigits, d, ddigits)
UINT32 *a, *b, *c, *d;
UINT32 cdigits, ddigits;
{
    UINT32 ci = 0 ;
    UINT32 *t = NULL;
    INT32 i =0 ;
    UINT32 cibits = 0, 
           j = 0, 
           s = 0;

    if((NULL == a)||(NULL == b)||(NULL ==c)||(NULL == d))
    {
        return RET_FAILURE;
    }
    t=(UINT32 *)malloc(MAX_ALI_DIGITS*4);
    if(NULL == t)
    {
        return RET_FAILURE;
    }
    /* Store b, b^2 mod d, and b^3 mod d.
    */
    ali_assign (bpower[0], b, ddigits);
    ali_mod_mult (bpower[1], bpower[0], b, d, ddigits);
    ali_mod_mult (bpower[2], bpower[1], b, d, ddigits);

    ali_assign_zero(t,ddigits);
    t[0] = 1;

    cdigits = ali_digits (c, cdigits);
    for (i = cdigits - 1; i >= 0; i--) 
    {
            ci = c[i];
            cibits = ALI_DIGIT_BITS;

        /* Scan past leading zero bits of most significant digit.*/
        if (i == (INT32)(cdigits - 1)) 
        {
            while (!DIGIT_2MSB (ci)) 
            {
                ci <<= 2;
                cibits -= 2;
            }
        }

        for (j = 0; j < cibits; j += 2, ci <<= 2) 
        {
        /* Compute t = t^4 * b^s mod d, where s = two MSB's of ci.
             */
            ali_mod_mult (t, t, t, d, ddigits);
            ali_mod_mult (t, t, t, d, ddigits);
            if ((s = DIGIT_2MSB (ci)) != 0)
            {
                ali_mod_mult (t, t, bpower[s-1], d, ddigits);
            }
        }
    }

    ali_assign (a, t, ddigits);

    /* Zeroize potentially sensitive information.
    */
    r_memset ((UINT8 *)bpower, 0, sizeof (bpower));
    r_memset ((UINT8 *)t, 0, MAX_ALI_DIGITS*4);
    free(t);
		return RET_SUCCESS ;
}

/* Raw RSA public-key operation. Output has same length as modulus.

     Requires input < modulus.
*/
RET_CODE ali_rsapublicfunc(output, outputlen, input, inputlen, publickey)
UINT8 *output;          /* output block */
UINT32 *outputlen;        /* length of output block */
UINT8 *input;           /* input block */
UINT32 inputlen;          /* length of input block */
R_RSA_PUBLIC_KEY *publickey;    /* RSA public key */
{
    UINT32 *c = NULL, 
           *e = NULL, 
           *m = NULL ,
           *n = NULL;
    UINT32 edigits = 0, 
           ndigits = 0;
    RET_CODE ret = RET_FAILURE ;

    if((NULL == output)||(NULL == outputlen)||(NULL == input))
    {
        return RET_FAILURE;
    }
    c= (UINT32 *)malloc(MAX_ALI_DIGITS*4);
    if(NULL == c)
    {
        return RET_FAILURE ;
    }
    
    e= (UINT32 *)malloc(MAX_ALI_DIGITS*4);
    if(NULL == e)
    {
        free(c);
        c=NULL;
        return RET_FAILURE ;
    }
    
    m= (UINT32 *)malloc(MAX_ALI_DIGITS*4);
    if(NULL == m)
    {
        free(c);
        c=NULL;
        free(e);
        e=NULL;
        return RET_FAILURE ;
    }
    
    n= (UINT32 *)malloc(MAX_ALI_DIGITS*4);
    if(NULL == n)
    {
        free(c);
        c=NULL;
        free(e);
        e=NULL;
        free(m);
        m=NULL;
        return RET_FAILURE ;
    }


        /* decode the required RSA function input data */
    ali_decode(m, MAX_ALI_DIGITS, input, inputlen);
    ali_decode(n, MAX_ALI_DIGITS, publickey->modulus, ALI_RSA_MODULUS_LEN);
    ali_decode(e, MAX_ALI_DIGITS, publickey->exponent, ALI_RSA_MODULUS_LEN);

    ndigits = ali_digits(n, MAX_ALI_DIGITS);
    edigits = ali_digits(e, MAX_ALI_DIGITS);

    if(ali_cmp(m, n, ndigits) >= 0)
    {
        free(c);
        c=NULL;
        free(e);
        e=NULL;
        free(m);
        m=NULL;
        free(n);
        n=NULL;
        return RET_FAILURE;
    }

    *outputlen = (publickey->bits + 7) / 8;

    /* Compute c = m^e mod n.  To perform actual RSA calc.*/
    


    ret = ali_mod_exp (c, m, e, edigits, n, ndigits);
    if(ret != RET_SUCCESS)
    {
        free(c);
        c=NULL;
        free(e);
        e=NULL;
        free(m);
        m=NULL;
        free(n);
        n=NULL;
       return ret ;
    }

    /* encode output to standard form */
    ali_encode (output, *outputlen, c, ndigits);

    /* Clear sensitive information. */

    r_memset((UINT8 *)c, 0, MAX_ALI_DIGITS*4);
    r_memset((UINT8 *)m, 0, MAX_ALI_DIGITS*4);
    free(c);
    c=NULL;
    free(e);
    e=NULL;
    free(m);
    m=NULL;
    free(n);
    n=NULL;

    return RET_SUCCESS;
}

/*
 *Add the padding for signature, 
 *the padding format means the algorithm of SHA is SHA256 
 */
static UINT8 digest_info_b[] =
{
    0x05,0x00,0x04,0x20, 
};

static UINT8 digest_info_a[] =
{
    0x00,0x30,0x31,0x30,0x0d,0x06,0x09,0x60,
    0x86,0x48,0x01,0x65,0x03,0x04,0x02,
};


static void __aliasix_sha_result_conversion(UINT8 *sha_padding, UINT8 *result)
{
    UINT32 i = 0;
    void *dummy =0;

    sha_padding[0]=0;
    sha_padding[1]=0x1;

    for(i=2;i<204;i++)
    {
        sha_padding[i]=0xff;
    }

    dummy =memcpy(&sha_padding[i],digest_info_a,sizeof(digest_info_a));
    i+=sizeof(digest_info_a);
    sha_padding[i++]=0x1;
    dummy=memcpy(&sha_padding[i],digest_info_b,sizeof(digest_info_b));
    i+=sizeof(digest_info_b);
    dummy=memcpy(&sha_padding[i],result,ALIASIX_HASH_DIGEST_LEN);
    return;
}
/*
 * Verify the given data with Redundant checking
 */
RET_CODE ali_rsa_result_compare(UINT8 *rsa_result, void *sha_result)
{
    UINT32 i = 0,
           j = 0;
    
    UINT8  sha_padding[ALIASIX_RSA_COMPARE_LEN];

    __aliasix_sha_result_conversion(sha_padding,sha_result);
    if(0 != memcmp((void *)rsa_result, 
                    (void *)sha_padding, 
                    ALIASIX_RSA_COMPARE_LEN))
    {
            return RET_FAILURE;
    }

    /*Redundant check*/
    DOUBLE_FOR(i,j,ALIASIX_RSA_COMPARE_LEN,4)
    {
        if (*(volatile UINT32*)((UINT32)rsa_result+i) !=
                *(volatile UINT32 *)((UINT32)sha_padding+i))
        {     
            return RET_FAILURE;
        }
    }   
    
    return RET_SUCCESS;
}

