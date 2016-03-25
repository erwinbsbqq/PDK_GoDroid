/*
 *  ALi advanced security module
 *
 *  This file contains the ALi advanced verification implementations.
 *
 *  Author:
 *	Zhao Owen <owen.zhao@alitech.com>
 *
 *  Copyright (C) 2011 Zhao Owen <owen.zhao@alitech.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2,
 *      as published by the Free Software Foundation.
 */

#include "aliasix_rsa.h"
#include "aliasix_sha.h"

/* Public key */
u8 aliasix_rsa_public_modulus[RSA_MAX_RSA_MODULUS_LEN];

u8 aliasix_rsa_public_exponent[RSA_MAX_RSA_MODULUS_LEN];

/*
 * Useful MACROs
 */
#define ALIASIX_RSA_ATOH(result, str, condition, idx) \
    do { \
        result <<= 4; \
        if ((str[idx + 2] - '0') <= 9) \
	{ \
            result |= ((str[idx + 2] - '0') & 0x0000000F); \
	} \
        else \
	{ \
            result |= ((str[idx + 2] - 'a' + 10) & 0x0000000F); \
	} \
        idx++; \
    } while (condition)

/* Structure to accept data from U-Boot */
typedef struct _aliasix_rsa_public_key {
  unsigned int bits;                                    /* length in bits of modulus */
  unsigned char modulus[RSA_MAX_RSA_MODULUS_LEN];       /* modulus */
  unsigned char exponent[RSA_MAX_RSA_MODULUS_LEN];      /* public exponent */
} aliasix_rsa_public_key;


/*
 * To init rsa public key
 * \@ param format:
 */
  int  aliasix_rsa_setup(void * addr)
{
  
    aliasix_rsa_public_key *rsa_pub_key = NULL;

    rsa_pub_key = (aliasix_rsa_public_key *)addr;
    memcpy(aliasix_rsa_public_modulus, rsa_pub_key->modulus, RSA_MAX_RSA_MODULUS_LEN);
    memcpy(aliasix_rsa_public_exponent, rsa_pub_key->exponent, RSA_MAX_RSA_MODULUS_LEN);
    return 1;
}

