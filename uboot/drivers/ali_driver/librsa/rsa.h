#ifndef _CRYPTO_RSA_H
#define _CRYPTO_RSA_H
#include <asm/errno.h>
/*
 * Useful macros
 */
/* RSA MACROs */
#define RSA_DIGIT_BITS 32
#define RSA_DIGIT_LEN (RSA_DIGIT_BITS / 8)
/* Maximum length in digits */
#define RSA_MAX_RSA_MODULUS_BITS 2048
#define RSA_MAX_RSA_MODULUS_LEN ((RSA_MAX_RSA_MODULUS_BITS + 7) / 8)
#define RSA_MAX_DIGITS \
  ((RSA_MAX_RSA_MODULUS_LEN + RSA_DIGIT_LEN - 1) / RSA_DIGIT_LEN + 1)


#define CONFIG_RSA_AUX_OPERAND_SIZE 128

/* 
 * struct rsa_op: multi-precision integer operand
 * @data: array that holds the number absolute value
 * @sign: 1 for negative, 0 for positive
 * @size: significant number limbs
 * @limbs: allocated limbs (sizeof data)
 */
#ifndef bool
#define bool int
enum {
	false	= 0,
	true	= 1
};
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef u8
#define u8 unsigned char
#endif 

#ifndef u16
#define u16 unsigned short
#endif 


typedef struct rsa_op {
	u32 *data;
	int sign;
	int size;
	int limbs;
} rsa_op;

int	    rsa_op_alloc(rsa_op **, int);
void	rsa_op_free(rsa_op *);
int 	rsa_op_init(rsa_op **, u8 *, u32, u32);
int 	rsa_op_set(rsa_op **, u8 *, u32);
int	    rsa_op_copy(rsa_op **, rsa_op *);
void	rsa_op_print(rsa_op *, bool);
int 	rsa_cipher(rsa_op **, rsa_op *, rsa_op *, rsa_op *);
void    rsa_decode (rsa_op *, unsigned int , unsigned char *, unsigned int);

#endif
