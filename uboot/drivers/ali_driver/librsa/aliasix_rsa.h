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

#ifndef _SECURITY_ALIASIX_RSA_H_
#define _SECURITY_ALIASIX_RSA_H_

/*
 * ALiasix header
 */
#include "aliasix.h"
#include "rsa.h"

/* Maximum length in digits */
#define ALIASIX_MAX_DIGITS RSA_MAX_DIGITS

/* Macros of the RSA elements */
#define ALIASIX_RSA_MAX_RSA_MODULUS_BITS 2048
#define ALIASIX_MAX_RSA_MODULUS_LEN RSA_MAX_RSA_MODULUS_LEN

extern u8 aliasix_rsa_public_modulus[];
extern u8 aliasix_rsa_public_exponent[];

#endif
