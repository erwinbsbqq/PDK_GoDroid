#ifndef _RSA_VERIFY_H_
#define _RSA_VERIFY_H_

int ali_rsa_verification(unsigned char *addr, unsigned char *sign, \
                    const unsigned int length, unsigned char *pub_address);
#endif
