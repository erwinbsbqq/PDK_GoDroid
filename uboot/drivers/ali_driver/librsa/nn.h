#ifndef _NN_H_
#define _NN_H_
#include <ali/basic_types.h>

/* Macros. */
#define LOW_HALF(x) ((x) & MAX_ALI_HALF_DIGIT)
#define HIGH_HALF(x) (((x) >> ALI_HALF_DIGIT_BITS) & MAX_ALI_HALF_DIGIT)
#define TO_HIGH_HALF(x) (((UINT32)(x)) << ALI_HALF_DIGIT_BITS)
#define DIGIT_MSB(x) (unsigned int)(((x) >> (ALI_DIGIT_BITS - 1)) & 1)
#define DIGIT_2MSB(x) (unsigned int)(((x) >> (ALI_DIGIT_BITS - 2)) & 3)
#define ALI_ASSIGN_DIGIT(a, b, digits) {ALI_AssignZero (a, digits); a[0] = b;}

#define ALI_RSA_MODULUS_BITS 2048
#define ALI_RSA_MODULUS_LEN ((ALI_RSA_MODULUS_BITS + 7) / 8)

/* Length of digit in bits */
#define ALI_DIGIT_BITS 32
#define ALI_HALF_DIGIT_BITS 16
/* Length of digit in bytes */
#define ALI_DIGIT_LEN (ALI_DIGIT_BITS / 8)
/* Maximum length in digits */
#define MAX_ALI_DIGITS ((ALI_RSA_MODULUS_LEN + ALI_DIGIT_LEN - 1) / ALI_DIGIT_LEN + 1)
/* Maximum digits */
#define MAX_ALI_DIGIT 0xffffffff
#define MAX_ALI_HALF_DIGIT 0xffff

typedef struct {
  UINT32 bits;                           /* length in bits of modulus */
  UINT8 modulus[ALI_RSA_MODULUS_LEN];  /* modulus */
  UINT8 exponent[ALI_RSA_MODULUS_LEN]; /* public exponent */
} R_RSA_PUBLIC_KEY;

#define ALIASIX_HASH_DIGEST_LEN 			  32
#define ALIASIX_RSA_COMPARE_LEN 			  256


#endif /* _NN_H_ */
