/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2009 Copyright (C)
 *  (C)
 *  File: ali_video.c
 *  (I)
 *  Description: ali video player
 *  (S)
 *  History:(M)
 *          Date                    Author          Comment
 *          ====                    ======      =======
 * 0.       2009.12.24          Sam         Create
 ****************************************************************************/
#include "ali_video.h"

int ali_video_dbg_on = 0;
int ali_video_ape_dbg_on = 0;
struct ali_video_info *ali_video_priv;

#define ALIDES_PRINTF(...) do{}while(0)
//#define ALIDES_PRINTF printk

#define MEM_ADDR_256_ANIMA  0xa8800000
#define MEM_ADDR_128_ANIMA  0xa3a00000
#define MEM_ADDR_16S_ANIMA  0xa5780000

#define DES_MODE_ENCRYPT        0
#define DES_MODE_DECRYPT        1

#define MAX_TABLE 16
#define TABLE_LEN  16+4+16
static const unsigned char kkytgi_gjjddj[128] = "@ALI^OS#JMSR$DEF";

static const unsigned char dfw_faga_mbnf[MAX_TABLE][TABLE_LEN] =
{
    {"%^&DFLJNSN)(SDFW"},
    {"~!#SFAKW(&SDKKHS"},
    {"BMD23KS)FK$%2DKG"},
    {"GS*^DJGG(92FHSKF"},
    {")(FHAS7(KSF$#SFS"},
    {"$KIG&$#GJ(IJ*GJK"},
    {"P*^%JHLK^$JHFK(&"},
    {"F_*%^HFSK*&%FJDG"},
    {"*&AFWK&%SKFHSF2F"},
    {"97KL(&^KFKA90FSF"},
    {")*^DFLI2JSFA09LH"},
    {"AF^DL02JLS-82LF0"},
    {"NTMVNSOOW(^LDFPA"},
    {")*^ADFJW0JLSFSG3"},
    {"QFJP^$(SLNBMSJEO"},
    {"CNI932*^LSBBAJVL"}
};

static long long g_arrayMask[64] = 
{
    0x0000000000000001, 0x0000000000000002, 0x0000000000000004, 0x0000000000000008,
    0x0000000000000010, 0x0000000000000020, 0x0000000000000040, 0x0000000000000080,
    0x0000000000000100, 0x0000000000000200, 0x0000000000000400, 0x0000000000000800,
    0x0000000000001000, 0x0000000000002000, 0x0000000000004000, 0x0000000000008000,
    0x0000000000010000, 0x0000000000020000, 0x0000000000040000, 0x0000000000080000,
    0x0000000000100000, 0x0000000000200000, 0x0000000000400000, 0x0000000000800000,
    0x0000000001000000, 0x0000000002000000, 0x0000000004000000, 0x0000000008000000,
    0x0000000010000000, 0x0000000020000000, 0x0000000040000000, 0x0000000080000000,
    0x0000000100000000, 0x0000000200000000, 0x0000000400000000, 0x0000000800000000,
    0x0000001000000000, 0x0000002000000000, 0x0000004000000000, 0x0000008000000000,
    0x0000010000000000, 0x0000020000000000, 0x0000040000000000, 0x0000080000000000,
    0x0000100000000000, 0x0000200000000000, 0x0000400000000000, 0x0000800000000000,
    0x0001000000000000, 0x0002000000000000, 0x0004000000000000, 0x0008000000000000,
    0x0010000000000000, 0x0020000000000000, 0x0040000000000000, 0x0080000000000000,
    0x0100000000000000, 0x0200000000000000, 0x0400000000000000, 0x0800000000000000,
    0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000
};

static int g_arrayIP[64] = 
{
    57, 49, 41, 33, 25, 17,  9,  1, 
    59, 51, 43, 35, 27, 19, 11,  3, 
    61, 53, 45, 37, 29, 21, 13,  5, 
    63, 55, 47, 39, 31, 23, 15,  7, 
    56, 48, 40, 32, 24, 16,  8,  0, 
    58, 50, 42, 34, 26, 18, 10,  2, 
    60, 52, 44, 36, 28, 20, 12,  4, 
    62, 54, 46, 38, 30, 22, 14,  6 
};

static int g_arrayE[64] = 
{
    31,  0,  1,  2,  3,  4, -1, -1, 
     3,  4,  5,  6,  7,  8, -1, -1, 
     7,  8,  9, 10, 11, 12, -1, -1, 
    11, 12, 13, 14, 15, 16, -1, -1, 
    15, 16, 17, 18, 19, 20, -1, -1, 
    19, 20, 21, 22, 23, 24, -1, -1, 
    23, 24, 25, 26, 27, 28, -1, -1, 
    27, 28, 29, 30, 31, 30, -1, -1
};

static char g_matrixNSBox[8][64] = 
{
    {
        14,  4,  3, 15,  2, 13,  5,  3,
        13, 14,  6,  9, 11,  2,  0,  5,
         4,  1, 10, 12, 15,  6,  9, 10,
         1,  8, 12,  7,  8, 11,  7,  0,
         0, 15, 10,  5, 14,  4,  9, 10,
         7,  8, 12,  3, 13,  1,  3,  6,
        15, 12,  6, 11,  2,  9,  5,  0,
         4,  2, 11, 14,  1,  7,  8, 13
    },
    {
        15,  0,  9,  5,  6, 10, 12,  9,
         8,  7,  2, 12,  3, 13,  5,  2,
         1, 14,  7,  8, 11,  4,  0,  3,
        14, 11, 13,  6,  4,  1, 10, 15,
         3, 13, 12, 11, 15,  3,  6,  0,
         4, 10,  1,  7,  8,  4, 11, 14,
        13,  8,  0,  6,  2, 15,  9,  5,
         7,  1, 10, 12, 14,  2,  5,  9
    },
    {
        10, 13,  1, 11,  6,  8, 11,  5,
         9,  4, 12,  2, 15,  3,  2, 14,
         0,  6, 13,  1,  3, 15,  4, 10,
        14,  9,  7, 12,  5,  0,  8,  7,
        13,  1,  2,  4,  3,  6, 12, 11,
         0, 13,  5, 14,  6,  8, 15,  2,
         7, 10,  8, 15,  4,  9, 11,  5,
         9,  0, 14,  3, 10,  7,  1, 12
    },
    {
         7, 10,  1, 15,  0, 12, 11,  5,
        14,  9,  8,  3,  9,  7,  4,  8,
        13,  6,  2,  1,  6, 11, 12,  2,
         3,  0,  5, 14, 10, 13, 15,  4,
        13,  3,  4,  9,  6, 10,  1, 12,
        11,  0,  2,  5,  0, 13, 14,  2,
         8, 15,  7,  4, 15,  1, 10,  7,
         5,  6, 12, 11,  3,  8,  9, 14
    },
    {
         2,  4,  8, 15,  7, 10, 13,  6,
         4,  1,  3, 12, 11,  7, 14,  0,
        12,  2,  5,  9, 10, 13,  0,  3,
         1, 11, 15,  5,  6,  8,  9, 14,
        14, 11,  5,  6,  4,  1,  3, 10,
         2, 12, 15,  0, 13,  2,  8,  5,
        11,  8,  0, 15,  7, 14,  9,  4,
        12,  7, 10,  9,  1, 13,  6,  3
    },
    {
        12,  9,  0,  7,  9,  2, 14,  1,
        10, 15,  3,  4,  6, 12,  5, 11,
         1, 14, 13,  0,  2,  8,  7, 13,
        15,  5,  4, 10,  8,  3, 11,  6,
        10,  4,  6, 11,  7,  9,  0,  6,
         4,  2, 13,  1,  9, 15,  3,  8,
        15,  3,  1, 14, 12,  5, 11,  0,
         2, 12, 14,  7,  5, 10,  8, 13
    },
    {
         4,  1,  3, 10, 15, 12,  5,  0,
         2, 11,  9,  6,  8,  7,  6,  9,
        11,  4, 12, 15,  0,  3, 10,  5,
        14, 13,  7,  8, 13, 14,  1,  2,
        13,  6, 14,  9,  4,  1,  2, 14,
        11, 13,  5,  0,  1, 10,  8,  3,
         0, 11,  3,  5,  9,  4, 15,  2,
         7,  8, 12, 15, 10,  7,  6, 12
    },
    {
        13,  7, 10,  0,  6,  9,  5, 15,
         8,  4,  3, 10, 11, 14, 12,  5,
         2, 11,  9,  6, 15, 12,  0,  3,
         4,  1, 14, 13,  1,  2,  7,  8,
         1,  2, 12, 15, 10,  4,  0,  3,
        13, 14,  6,  9,  7,  8,  9,  6,
        15,  1,  5, 12,  3, 10, 14,  5,
         8,  7, 11,  0,  4, 13,  2, 11
    },
};

static int g_arrayP[32] = 
{
    15,  6, 19, 20, 28, 11, 27, 16, 
     0, 14, 22, 25,  4, 17, 30,  9, 
     1,  7, 23, 13, 31, 26,  2,  8, 
    18, 12, 29,  5, 21, 10,  3, 24
};

static int g_arrayIP_1[64] = 
{
    39,  7, 47, 15, 55, 23, 63, 31, 
    38,  6, 46, 14, 54, 22, 62, 30, 
    37,  5, 45, 13, 53, 21, 61, 29, 
    36,  4, 44, 12, 52, 20, 60, 28, 
    35,  3, 43, 11, 51, 19, 59, 27, 
    34,  2, 42, 10, 50, 18, 58, 26, 
    33,  1, 41,  9, 49, 17, 57, 25, 
    32,  0, 40,  8, 48, 16, 56, 24
};

static int g_arrayPC_1[56] = 
{
    56, 48, 40, 32, 24, 16,  8,
     0, 57, 49, 41, 33, 25, 17,
     9,  1, 58, 50, 42, 34, 26,
    18, 10,  2, 59, 51, 43, 35,
    62, 54, 46, 38, 30, 22, 14,
     6, 61, 53, 45, 37, 29, 21,
    13,  5, 60, 52, 44, 36, 28,
    20, 12,  4, 27, 19, 11,  3
};

static int g_arrayPC_2[64] = 
{
    13, 16, 10, 23,  0,  4, -1, -1,
     2, 27, 14,  5, 20,  9, -1, -1,
    22, 18, 11,  3, 25,  7, -1, -1,
    15,  6, 26, 19, 12,  1, -1, -1,
    40, 51, 30, 36, 46, 54, -1, -1,
    29, 39, 50, 44, 32, 47, -1, -1,
    43, 48, 38, 55, 33, 52, -1, -1,
    45, 41, 49, 35, 28, 31, -1, -1
};

static int g_arrayLs[16] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

static long long g_arrayLsMask[3] = 
{
    0x0000000000000000,
    0x0000000000100001,
    0x0000000000300003
};

/* Globle CRC select switch */
// #define MG_CRC_32_ARITHMETIC
#define MG_CRC_32_ARITHMETIC_CCITT
/* #define MG_CRC_24_ARITHMETIC_CCITT
 * #define MG_CRC_24_ARITHMETIC */

#ifdef MG_CRC_32_ARITHMETIC_CCITT
#define MG_CRC_32_BIT
#endif

#ifdef MG_CRC_32_ARITHMETIC
#define MG_CRC_32_BIT
#endif

#ifdef MG_CRC_24_ARITHMETIC_CCITT
#define MG_CRC_24_BIT
#endif

#ifdef MG_CRC_24_ARITHMETIC
#define MG_CRC_24_BIT
#endif

/* 0xbba1b5 = 1'1011'1011'1010'0001'1011'0101b
 <= x24+x23+x21+x20+x19+x17+x16+x15+x13+x8+x7+x5+x4+x2+1 */
#ifdef MG_CRC_24_ARITHMETIC_CCITT
#define MG_CRC_24_CCITT     0x00ddd0da
#endif

#ifdef MG_CRC_24_ARITHMETIC
#define MG_CRC_24           0x00ad85dd
#endif

/* 0x04c11db7 = 1'0000'0100'1100'0001'0001'1101'1011'0111b
 <= x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 */
#ifdef MG_CRC_32_ARITHMETIC_CCITT
#define MG_CRC_32_CCITT     0x04c11db7
#endif

#ifdef MG_CRC_32_ARITHMETIC
#define MG_CRC_32           0xedb88320
#endif

unsigned int MG_CRC_Table[256];
/*From "len" length "buffer" data get CRC, "crc" is preset value of crc*/
unsigned int MG_Compute_CRC(register unsigned int crc,
                        register unsigned char *bufptr,
                        register int len)
{
    register int i;

#ifdef MG_CRC_24_ARITHMETIC_CCITT
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)(*bufptr) << 16);
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x00800000)    /*Highest bit procedure*/
                crc = (crc << 1) ^ MG_CRC_24_CCITT;
            else
                crc <<= 1;
        }
    }
    return(crc & 0x00ffffff);  /*Get lower 24 bits FCS*/
#endif

#ifdef MG_CRC_24_ARITHMETIC
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)*bufptr;
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 1)             /*Lowest bit procedure*/
                crc = (crc >> 1) ^ MG_CRC_24;
            else
                crc >>= 1;
        }
    }
    return(crc & 0x00ffffff);  /*Get lower 24 bits FCS*/
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)(*bufptr) << 24;
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x80000000)    /*Highest bit procedure*/
                crc = (crc << 1) ^ MG_CRC_32_CCITT;
            else
                crc <<= 1;
        }
    }
    return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
#endif

#ifdef MG_CRC_32_ARITHMETIC
    while(len--)  /*Length limited*/
    {
        crc ^= (unsigned int)*bufptr;
        bufptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 1)             /*Lowest bit procedure*/
                crc = (crc >> 1) ^ MG_CRC_32;
            else
                crc >>= 1;
        }
    }
    return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
#endif
}

/*Setup fast CRC compute table*/
void MG_Setup_CRC_Table(void)
{
    register int count;
    unsigned char zero=0;

    for(count = 0; count <= 255; count++) /*Comput input data's CRC, from 0 to 255*/
    {
#ifdef MG_CRC_24_ARITHMETIC_CCITT
        MG_CRC_Table[count] = MG_Compute_CRC(count << 16,&zero,1);
#endif

#ifdef MG_CRC_24_ARITHMETIC
        MG_CRC_Table[count] = MG_Compute_CRC(count,&zero,1);
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
        MG_CRC_Table[count] = MG_Compute_CRC(count << 24,&zero,1);
#endif

#ifdef MG_CRC_32_ARITHMETIC
        MG_CRC_Table[count] = MG_Compute_CRC(count,&zero,1);
#endif
    }
}

/*Fast CRC compute*/
static unsigned int MG_Table_Driven_CRC(register unsigned int crc,
                const unsigned char *bufptr,
                register int len)
{
    register int i;

    for(i = 0; i < len; i++)
    {
#ifdef MG_CRC_24_ARITHMETIC_CCITT
        crc=(MG_CRC_Table[((crc >> 16) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0x00ffffff;
#endif

#ifdef MG_CRC_24_ARITHMETIC
        crc=(MG_CRC_Table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0x00ffffff;
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
        crc=(MG_CRC_Table[((crc >> 24) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0xffffffff;
#endif

#ifdef MG_CRC_32_ARITHMETIC
        crc=(MG_CRC_Table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0xffffffff;
#endif
    }
    
    return(crc);
}


#define ali_BitTransform(array, len, source, dest) \
{\
    long long bts = source;\
    int bti;\
    dest = 0;\
    for(bti = 0; bti < len; bti++)\
    {\
        if(array[bti] >= 0 && (bts & g_arrayMask[array[bti]]))\
        {\
            dest |= g_arrayMask[bti];\
        }\
    }\
}


static void ali_DESSubKeys(long long key, long long K[16], int mode)
{
    long long temp;
    int j;
    ali_BitTransform(g_arrayPC_1, 56, key, temp);
    for(j = 0; j < 16; j++)
    {
        {
            long long source = temp;
            temp = ((source & g_arrayLsMask[g_arrayLs[j]]) << (28 - g_arrayLs[j])) | ((source & ~g_arrayLsMask[g_arrayLs[j]]) >> g_arrayLs[j]);
        }
        ali_BitTransform(g_arrayPC_2, 64, temp, K[j]);
    }
    if(mode == DES_MODE_DECRYPT) 
    {
        long long t;
        for(j = 0; j < 8; j++)
        {
            t = K[j];
            K[j] = K[15 - j];
            K[15 - j] = t;
        }
    }
}

static long long ali_DES64(long long subkeys[16], long long data)
{
    static long long out;
    //static long long source;
    static long long L, R;
    static int * pSource;
    static char * pR;
    static int i;
    static int SOut;
    static int t;
    static int sbi;
    pSource = (int *)&out;
    pR = (char *)&R;
    {
        ali_BitTransform(g_arrayIP, 64, data, out);
    }
    {
        //source = out;
        for(i = 0; i < 16; i++)
        {
            R = pSource[1];
            {
                {
                    ali_BitTransform(g_arrayE, 64, R, R);
                }
                R ^= subkeys[i];
                {
                    SOut = 0;
                    for(sbi = 7; sbi >= 0; sbi--)
                    {
                        SOut <<= 4;
                        SOut |= g_matrixNSBox[sbi][pR[sbi]];
                    }
                    R = SOut;
                }
                {
                    ali_BitTransform(g_arrayP, 32, R, R);
                }
            }
            L = pSource[0];
            pSource[0] = pSource[1];
            pSource[1] = (int)(L ^ R);
        }
        {
            t = pSource[0];
            pSource[0] = pSource[1];
            pSource[1] = t;
        }
    }
    {
        ali_BitTransform(g_arrayIP_1, 64, out, out);
    }
    return out;
}

int module_func_sub_check(int func,int index)
{
    int ret=FALSE;
    int crc=0;
    int data_crc=0;
    unsigned char temp_buf[8];

    int decrypt_high=0;
    int decrypt_low=0;
    long long decrypt=0;
    long long key = 0x97530abcf83;
    long long subKey[16];

    if(index>=64)
    {
        ALIDES_PRINTF("%s index=%d >=64\n",__FUNCTION__,index);
        return FALSE;
    }

    data_crc=*(int *)(dfw_faga_mbnf[func]+16);
    //check not sign
    if(data_crc==0)
    {
        ALIDES_PRINTF("data_crc==0 failed!\n");
        return FALSE;
    }
    //check not sign    
    memset(temp_buf,0,8);
    if(memcmp(dfw_faga_mbnf[func]+16+4,temp_buf,8)==0)
    {
        ALIDES_PRINTF("have not sign!\n");
        return FALSE;
    }
    //check crc
    MG_Setup_CRC_Table();
    crc = MG_Table_Driven_CRC(0xFFFFFFFF, dfw_faga_mbnf[func]+16+4 ,8);
    
    ALIDES_PRINTF("crc=0x%x data_crc=0x%x\n",crc,data_crc);
    if(crc != data_crc)
    {
        ALIDES_PRINTF("crc=0x%x != data_crc=0x%x crc check failed!\n",crc,data_crc);
       // return FALSE;    
    }
    
    //decrypt
    ali_DESSubKeys(key, subKey, DES_MODE_DECRYPT);
    decrypt = ali_DES64(subKey, *(long long *)(dfw_faga_mbnf[func]+16+4));

    decrypt_high=decrypt >> 32;
    decrypt_low =decrypt & 0x00000000ffffffff;
    ALIDES_PRINTF("decrypt_high=0x%x decrypt_low=0x%x index=%d\n",decrypt_high,decrypt_low,index);

    if(index<=31)
    {
        if(decrypt_low & (1<<(index)))
            ret = TRUE;
        else
            ret = FALSE;      
    }
    else
    {
        if(decrypt_high & (1<<(index-32)))
            ret =  TRUE;
        else
            ret =  FALSE;      
    }

     ALIDES_PRINTF("%s ret=%d\n",__FUNCTION__,ret);
    
    return ret;

}


int module_func_sub_match(int func, int param)
{
    int ret=FALSE;
    #if 0
    if(param)
    {
        ret = module_attach(param);
        if(ret ==TRUE)
            ret = module_func_sub_do(func,0);    
    }
    else
    {
        ret = module_func_sub_do(func,0);   
    }
    #endif
    return ret;
}
//check or not, 0xa5 mean not check
//static unsigned char qdbjadfa3=0xa5;
static int module_func_sub_general(int func, int param)
{
    int ret=FALSE;
    int chip_id=0;
    int product_id=0;
  
    //for general subfunction, 1bit for one function limitation
    ALIDES_PRINTF("%s func=%d param=%d\n", __FUNCTION__,func,param);

    //param 0 is subfunction: check M3612 can use other S2
    if(param == 0)
    { 
        #if 0
        if(qdbjadfa3 == TRUE || qdbjadfa3 == FALSE)
            return qdbjadfa3;
        if(sys_ic_product_is_m3612())
        {
            ALIDES_PRINTF("%s product is M3612\n", __FUNCTION__);
            ret = module_func_sub_check(func,param);
               
        }
        else
            ret = FALSE;  

        qdbjadfa3 = ret; 
        #endif
    }
    //param 1 is subfunction: special PVR recording
    else if(param == 1)
    {
        #if 0
        chip_id = ali_sys_ic_get_chip_id();
        switch(chip_id)
        {
            case ALI_S3602F:
                product_id = sys_ic_get_c3603_product();
                ALIDES_PRINTF("Func%d param=%d ALI_S3602F product_id=%d\n",func,param,product_id);
                //M3601E M3606 M3606B M3606C M3606D M3601S ->0,2,3,4,5,16
                //if(product_id ==0 || (product_id>=2 && product_id<=5) ||product_id==16 )
                //M3601E M3606 M3601S ->0,2,16
                if(product_id ==0 || product_id ==2  ||product_id==16 )
                    ret = module_func_sub_check(func,param);
                break;
            case ALI_S3503:
                product_id = sys_ic_get_product_id();
                ALIDES_PRINTF("Func%d param=%d ALI_S3503 product_id=%d\n",func,param,product_id);
                //3511 3512 -> 1,2
                if(product_id ==1 || product_id ==2 )
                    ret = module_func_sub_check(func,param);
                break;
            default:
                ret = FALSE;
                break;
        }     
        #endif
    }
    //param 2 is subfunction: USB 3G
    else if(param == 2)
    {
        //not need to check IC or product
        ret = TRUE;//module_func_sub_check(func,param); //3G not be controlled after SDK11.0
    }
    //param 3 is subfunction: SAT2IP(B2S)
    else if(param == 3)
    {
        ret = module_func_sub_check(func,7);
#if 0        
        chip_id = sys_ic_get_chip_id();
        switch(chip_id)
        {
            case ALI_S3503:
                product_id = sys_ic_get_product_id();
                ALIDES_PRINTF("Func%d param=%d ALI_S3503 product_id=%d\n",func,param,product_id);
                //3511 -> 1
                if(product_id ==1)
                    ret = module_func_sub_check(func,param);
                break;
            default:
                ret = FALSE;
                break;    
        }
#endif        
    }
    //param 4 is subfunction:DLNA(S2B)
    else if(param == 4)
    {
        //ret = module_func_sub_check(func,param);
#if 1        
        chip_id = ali_sys_ic_get_chip_id();
        
        switch(chip_id)
        {
            case ALI_S3503:
                product_id = ali_sys_ic_get_product_id();
                ALIDES_PRINTF("Func%d param=%d ALI_S3503 product_id=%d\n",func,param,product_id);
                if(product_id == 3)
                {
                    ret = module_func_sub_check(func,8);
                }
                else
                {
                    ret = FALSE;
                }
                #if 0
                //M3510 M3511 M3516 -> 0,1,3
                if(product_id == 0)
                    ret = module_func_sub_check(func,4);
                else if(product_id == 1)
                    ret = module_func_sub_check(func,5);
                else if(product_id == 3)
                    ret = module_func_sub_check(func,6);
                #endif     
                break;
            case ALI_C3701:
                product_id = ali_sys_ic_get_product_id();
                ALIDES_PRINTF("Func%d param=%d ALI_3701c product_id=%d\n",func,param,product_id);
                if((product_id == 0x11) || (product_id == 0x2))
                {
                    ret = TRUE;
                }
                else
                {
                    ret = FALSE;
                }               
                break;
            default:
                ret = FALSE;
                break;    
        }
#endif        
    }
    // it's for other function, expand here
    else
        ret = FALSE;
    
    return ret;
}

static int module_func_sub(int func, int param)
{
    int ret=FALSE;
     switch(func)
    {
        case 0:
            //wifi
            //ret = module_func_sub_do(0,0);
            break;
        case 1:
            //inview
           //ret = module_func_sub_match(1,param);
            break;
        case 2:
            //general subfunction check
            ret = module_func_sub_general(2,param);
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            //not use now
            //ret = module_func_sub_match(func,param);
            break;  
        default:
            break;
    }
    
    return ret;
        
}

static int module_func(int func, int param)
{
    int ret= FALSE;

    if(memcmp(&kkytgi_gjjddj[0],&dfw_faga_mbnf[0][0],6) == 0)
    {
        return ret;
    }
    ret = module_func_sub(func,param);
    return ret;    
}


static int request_buf(void *handle, void **buf_start, int *buf_size, ali_vdeo_ctrl_blk *blk)
{
    struct ali_video_info *info = (struct ali_video_info *)handle;
    int ret = ALI_VIDEO_REQ_RET_ERROR;
    int work = 0;

    if(info->status == ALI_VIDEO_WORKING || info->status == ALI_VIDEO_PAUSED)
        work = 1;
    else
        work = 0;
    
    if(work){       
        ret = hld_decv_request_buf(info, NULL, buf_start, buf_size, blk);
    }

//EXIT:
    return ret;
}

static void update_buf(void *handle, int buf_size)
{
    hld_decv_update_buf((struct ali_video_info *)handle, NULL, buf_size);
}

static ssize_t ali_video_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return -1;
}

static ssize_t ali_video_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct ali_video_info *info = (struct ali_video_info *)file->private_data;
    void *req_buf = NULL;
    int req_size = count;
    int req_ret = 0;
    int ret = 0;

    VDEC_PRINTF("video %u write data %d\n", info->vdec_type, count);

    if(down_interruptible(&info->sem)){
        VDEC_PRINTF("video down sem fail\n");
        return -EINVAL; 
    }

    if(info->open_count <= 0)
    {
        VDEC_PRINTF("video is not opened\n");
        goto EXIT;
    }

START:
    req_ret = request_buf(info, &req_buf, &req_size, NULL); 
    if(req_ret == ALI_VIDEO_REQ_RET_OK) {       
        char *user_buf_ptr = (char*)buf, *req_buf_ptr = (char*)req_buf;
        void *tmp_buf = (void *)info->tmp_mem_addr;
        int tmp_size = (int)info->tmp_mem_size, tmp_malloc = 0, write_size = req_size;

        if(tmp_buf == NULL) {
            tmp_buf = kmalloc(write_size, GFP_KERNEL);
            if(tmp_buf == NULL) {
                VDEC_PRINTF("kmalloc request buf fail\n");
                ret = -EINVAL;
                goto EXIT;
            } else {
                tmp_size = write_size;
                tmp_malloc = 1;
            }
        }

        do {
            tmp_size = (tmp_size > write_size) ? write_size : tmp_size;
            
            if(copy_from_user(tmp_buf, user_buf_ptr, tmp_size))
                return -EFAULT;
#if defined(CONFIG_ALI_CHIP_M3921)		

#else
            __CACHE_FLUSH_ALI((unsigned long)tmp_buf, req_size);
#endif
            /* Main CPU should use below mempy to transfer the data to private memory */
            hld_dev_memcpy(req_buf_ptr, (void *)__VMTSALI(tmp_buf), tmp_size);
            //memcpy((void*)__VSTMALI(req_buf_ptr), tmp_buf, tmp_size);
            user_buf_ptr += tmp_size;
            req_buf_ptr  += tmp_size;
            write_size   -= tmp_size;
        } while(write_size > 0);

        if(tmp_malloc) {
            kfree(tmp_buf);
        }
        
        update_buf(info, req_size);
        ret += req_size;
        if(req_size < count) {
            count = req_size = count - req_size;
            buf += req_size;
            goto START;
        }
    }else if(req_ret == ALI_VIDEO_REQ_RET_ERROR) {
        ret = -EINVAL;
    }

EXIT:   
    up(&info->sem);
    
    return ret;
}

static int video_ioctl_internal(struct ali_video_info *info, unsigned int cmd, unsigned long arg)
{
#if 0
    mem_section_attr_t *p_mem_section_attr = NULL;
#endif
    int ret = 0;
    
    switch(cmd){
        case ALIVIDEOIO_VIDEO_STOP:
        {       
            if(info->status == ALI_VIDEO_WORKING || info->status == ALI_VIDEO_PAUSED){
                info->status = ALI_VIDEO_IDLE;
            }
            break;
        }
        case ALIVIDEOIO_VIDEO_PLAY:
        {       
            if(info->status == ALI_VIDEO_IDLE){
                info->status = ALI_VIDEO_WORKING;
                VDEC_PRINTF("start ali video player done\n");
            }
            break;
        }
        case ALIVIDEOIO_GET_OUT_INFO:
        {
            struct ali_video_out_info_pars pars;

            memset((void *)&pars, 0, sizeof(pars));
            
            pars.started = (info->status == ALI_VIDEO_IDLE) ? 0 : 1;
            if(pars.started == 1)
                hld_decv_get_out_info(info, &pars);                 

            if(copy_to_user((void *)arg, &pars, sizeof(pars)))
                return -EFAULT;
            break;
        }
        case ALIVIDEOIO_RPC_OPERATION:
        {
            struct ali_video_rpc_pars pars;
            int i = 0;

            if(copy_from_user((void *)&pars, (void *)arg, sizeof(pars)))
                return -EFAULT;
            for(i = 0; i < pars.arg_num; i++) {
                info->rpc_arg_size[i] = pars.arg[i].arg_size;               
                if((info->rpc_arg_size[i] > 0) && (pars.arg[i].arg != NULL)) {     
                    if(info->rpc_arg[i] == NULL || info->rpc_arg_size[i] > MAX_VIDEO_RPC_ARG_SIZE) {
                        VDEC_PRINTF("allocate rpc arg buf fail\n");
                        ret = -ENOMEM;
                        goto RPC_EXIT;          
                    }

                    if(copy_from_user((void *)info->rpc_arg[i], pars.arg[i].arg, info->rpc_arg_size[i]))
                        return -EFAULT;
                }
            }

            ret = hld_decv_rpc_operation(info, pars.API_ID);
            
RPC_EXIT:           
            for(i = 0;i < pars.arg_num;i++) {
                if((info->rpc_arg_size[i] > 0) && (pars.arg[i].arg != NULL)) {
                    if(pars.arg[i].out) {
                        if(copy_to_user(pars.arg[i].arg, (void *)info->rpc_arg[i], info->rpc_arg_size[i]))
                            return -EFAULT;
                    }
                }           
            }
            
            break;
        }
        case ALIVIDEOIO_SET_SOCK_PORT_ID:
        {
            info->dst_pid = (int)arg;
            hld_decv_rpc_init(info);
            break;
        }
        case ALIVIDEOIO_GET_MEM_INFO:
        {
            struct ali_video_mem_info mem_info;
            mem_info.mem_size = __G_ALI_MM_VIDEO_SIZE;
            mem_info.mem_start = (void *)(__G_ALI_MM_VIDEO_START_ADDR & 0x1FFFFFFF);
            mem_info.priv_mem_size = 0;
            mem_info.priv_mem_start = NULL;
            mem_info.mp_mem_size = __G_ALI_MM_APE_MEM_SIZE;
            mem_info.mp_mem_start = (void *)(__G_ALI_MM_APE_MEM_START_ADDR & 0x1FFFFFFF);
            if(copy_to_user((void *)arg, (void *)&mem_info, sizeof(struct ali_video_mem_info)))
                return -EFAULT;
            break;
        }
        case ALIVIDEOIO_SET_MODULE_INFO:
        {
            int fun;
            int param;

            fun = ((int)arg >> 16) & 0xFFFF;
            param = (int)arg & 0xFFFF;
            ret = module_func(fun,param);
            break;
        }
        
        case ALIVIDEOIO_GET_BOOTMEDIA_INFO:
        {
            unsigned int boot_media_flag;
            unsigned char user_flag = 0;

            boot_media_flag = (*(volatile unsigned int *)(0xa4000000 - 0x1000 + 332)); // 332 = 288 + 20 + 8 + 16
            if((boot_media_flag == 0xFF55CC88) || (boot_media_flag == 0))
            {
                user_flag = 0x88;

                hld_decv_fb_cost_down(info, 0);

                printk("<0>""boot media done or no boot media\n");
            }

            //printk("<0>""%s : boot flag 0x%08x user flag %d\n", __FUNCTION__, boot_media_flag, user_flag);			
            if(copy_to_user((void *)arg, (void *)&user_flag, sizeof(unsigned char)))
                return -EFAULT;
            break;        
        }
        
        case ALIVIDEOIO_GET_BOOTMEDIA_TIME:
        {
            #if 0
            unsigned int boot_media_flag;
            unsigned int boot_media_addr;
            unsigned int *boot_media;
            p_mem_section_attr = (mem_section_attr_t *)get_mem_section_attr();
            if(p_mem_section_attr != NULL)
            {
                if(p_mem_section_attr->total_mem_size == 0xAFFFFFFF)
                {
                    if(p_mem_section_attr->see_mem_size == 0x01000000)
                        boot_media_addr = MEM_ADDR_16S_ANIMA;
                    else
                        boot_media_addr = MEM_ADDR_256_ANIMA;
                }
                else if(p_mem_section_attr->total_mem_size == 0x7FFFFFF)
                {
                    if(p_mem_section_attr->see_mem_size == 0x01000000)
                        boot_media_addr = MEM_ADDR_16S_ANIMA;
                    else
                        boot_media_addr = MEM_ADDR_128_ANIMA;
                }
                else
                    boot_media_addr = MEM_ADDR_128_ANIMA;
            }
            else
            {
                boot_media_addr = MEM_ADDR_128_ANIMA;
            }
            boot_media = boot_media_addr;
            boot_media_flag = ((unsigned int)*(boot_media + 3));
            copy_to_user((void *)arg, (void *)&boot_media_flag, sizeof(unsigned int));
            break;
            #endif
        }
        case ALIVIDEOIO_ENABLE_DBG_LEVEL:
		{
			UINT32 *rpc_par = (UINT32 *)info->rpc_arg[0];
			
			if(arg == 2)
			{
				rpc_par[0] = 1;				
				rpc_par[1] = 1;
				rpc_par[2] = 0;				
				hld_decv_rpc_operation(info, RPC_VIDEO_SET_DBG_FLAG);
			}
			else if(arg == 1)
			{
				ali_video_dbg_on = 1;
			}
			
			break;
		}
		case ALIVIDEOIO_DISABLE_DBG_LEVEL:
		{
			UINT32 *rpc_par = (UINT32 *)info->rpc_arg[0];
						
			if(arg == 2)
			{
				rpc_par[0] = 1;				
				rpc_par[1] = 0;
				rpc_par[2] = 0;
				hld_decv_rpc_operation(info, RPC_VIDEO_SET_DBG_FLAG);				
			}
			else if(arg == 1)
			{
				ali_video_dbg_on = 0;
			}
			
			break;
		}
        case ALIVIDEOIO_SET_CTRL_BLK_INFO:
        {
            if(copy_from_user((void *)&info->ctrl_blk, (void *)arg, sizeof(info->ctrl_blk)))
                return -EFAULT;
            info->ctrl_blk_enable = 1;
            break;
        }
        case ALIVIDEOIO_SET_APE_DBG_MODE:
        {
            if(arg)
                ali_video_ape_dbg_on = 1;
            else
                ali_video_ape_dbg_on = 0;

            break;
        }
        case ALIVIDEOIO_GET_APE_DBG_MODE:
        {
            if(!arg)
            break;

            if(copy_to_user((void *)arg, (void *)(&ali_video_ape_dbg_on), sizeof(int)))
                return -EFAULT;
            break;
        }
#if defined(CONFIG_ALI_CHIP_M3921)	
        case ALIVIDEO_ROTATION:
        {
            //extern unsigned long __G_ALI_MM_FB0_SIZE;
            extern unsigned long __G_ALI_MM_FB0_START_ADDR;
            extern unsigned long g_fb0_max_width;

            struct rotation_rect rot_rect;
            void *dst = (void *)__G_ALI_MM_FB0_START_ADDR;
            void *src = NULL;
            int pitch = 0;
            int i = 0;
            	
            if(copy_from_user(&rot_rect, (struct rotation_rect *)arg, sizeof(struct rotation_rect)))
                return -EFAULT;

            dst += 100 * g_fb0_max_width * 4 + 400;
            src =(void*) __VSTMALI(rot_rect.src_y_addr);
            pitch = rot_rect.width * 4;
            for(i = 0;i < rot_rect.height;i++)
            {
            memcpy(dst, src, pitch);

            src += pitch;
            dst += g_fb0_max_width * 4;				
            }
            				
            break;
        }
#else						
        case ALIVIDEO_ROTATION:
        {
        	struct rotation_rect rot_rect;
        	int img_width, img_height;
        	int i;
        	unsigned char *dst_y_addr;
        	unsigned char *dst_c_addr;
        	//unsigned int start_time, end_time;

                if(copy_from_user(&rot_rect, (struct rotation_rect *)arg, sizeof(struct rotation_rect)))
                    return -EFAULT;
                //img_width = ( rot_rect.width + 31 ) & 0xFFFFFFE0;
                //img_height = ( rot_rect.height + 15) & 0xFFFFFFF0;
                img_width = rot_rect.width;
                img_height = rot_rect.height;
                rot_rect.src_y_addr = (unsigned char*)(((unsigned int)rot_rect.src_y_addr & 0xFFFFFFF) | 0x80000000);
                rot_rect.src_c_addr = (unsigned char*)(((unsigned int)rot_rect.src_c_addr & 0xFFFFFFF ) | 0x80000000);
                //start_time = osal_get_tick();
                //vdec_cache_flush(rot_rect.src_y_addr, img_width*img_height);
                //vdec_cache_flush(rot_rect.src_c_addr, img_width*img_height/2);

                dst_y_addr = (unsigned char*)(((unsigned int)rot_rect.dst_y_addr & 0xFFFFFFF) | 0x80000000);
                dst_c_addr = (unsigned char*)(((unsigned int)rot_rect.dst_c_addr & 0xFFFFFFF) | 0x80000000);
                fb_rotation(rot_rect.src_y_addr, rot_rect.src_c_addr, dst_y_addr, dst_c_addr, rot_rect.width, rot_rect.height, rot_rect.angle);
                dma_cache_inv((unsigned int)dst_y_addr, img_width*img_height);
                dma_cache_inv((unsigned int)dst_c_addr, img_width*img_height/2);
                //end_time = osal_get_tick();
                if(copy_to_user((struct rotation_rect *)arg, &rot_rect, sizeof(struct rotation_rect)))
                    return -EFAULT;
                break;
        }
#endif		

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

static int ali_video_mp_ioctl(struct ali_video_info *info, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    
    switch(cmd)
    {
        case VDECIO_MP_INITILIZE:
        {
            struct vdec_mp_init_param init_param;
            
            ret = copy_from_user(&init_param, (void __user *)arg, sizeof(init_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            ret = hld_decv_mp_init(info, &init_param);
            break;
        }

        case VDECIO_MP_RELEASE:
        {
            struct vdec_mp_rls_param rls_param;
            
            ret = copy_from_user(&rls_param, (void __user *)arg, sizeof(rls_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            ret = hld_decv_mp_rls(info, &rls_param);
            break;
        }

        case VDECIO_MP_FLUSH:
        {
            struct vdec_mp_flush_param flush_param;

            ret = copy_from_user(&flush_param, (void __user *)arg, sizeof(flush_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            ret = hld_decv_mp_flush(info, &flush_param);
            break;
        }

        case VDECIO_MP_EXTRA_DATA:
        {
            struct vdec_mp_extra_data extra_data_param;
            unsigned char *extra_data = NULL;

            ret = copy_from_user(&extra_data_param, (void __user *)arg, sizeof(extra_data_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            extra_data = kmalloc(extra_data_param.extra_data_size, GFP_KERNEL);
            if(extra_data == NULL)
            {
                ret = -EINVAL;
                break;
            }
            
            ret = copy_from_user(extra_data, (void __user *)(extra_data_param.extra_data), \
                                 extra_data_param.extra_data_size);
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            extra_data_param.extra_data = extra_data;
            ret = hld_decv_mp_extra_data(info, &extra_data_param);
            kfree(extra_data);
            break;
        }

        case VDECIO_MP_GET_STATUS:
        {
            struct vdec_decore_status decore_status;
            
            ret = copy_from_user(&decore_status, (void __user *)arg, sizeof(decore_status));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            
            ret = hld_decv_mp_get_status(info, &decore_status);

            ret = copy_to_user((void __user *)arg, &decore_status, sizeof(decore_status));
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }

        case VDECIO_MP_PAUSE:
        {
            struct vdec_mp_pause_param pause_param;
            
            ret = copy_from_user(&pause_param, (void __user *)arg, sizeof(pause_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            ret = hld_decv_mp_pause(info, &pause_param);
            break;
        }

        case VDECIO_MP_SET_SBM_IDX:
        {
            struct vdec_mp_sbm_param sbm_param;

            ret = copy_from_user(&sbm_param, (void __user *)arg, sizeof(sbm_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            ret = hld_decv_mp_set_sbm(info, &sbm_param);
            break;          
        }

        case VDECIO_MP_SET_SYNC_MODE:
        {
            struct vdec_mp_sync_param sync_param;

            ret = copy_from_user(&sync_param, (void __user *)arg, sizeof(sync_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            ret = hld_decv_mp_set_sync_mode(info, &sync_param);
            break;          
        }

        case VDECIO_MP_SET_DISPLAY_RECT:
        {
            struct vdec_display_rect display_rect;

            ret = copy_from_user(&display_rect, (void __user *)arg, sizeof(display_rect));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            ret = hld_decv_mp_set_display_rect(info, &display_rect);
            break;  
        }

        case VDECIO_MP_SET_QUICK_MODE:
        {
            unsigned long quick_mode = arg;

            ret = hld_decv_mp_set_quick_mode(info, quick_mode);
            break;  
        }

        case VDECIO_MP_CAPTURE_FRAME:
        {
            struct vdec_picture picture;

            ret = copy_from_user(&picture, (void __user *)arg, sizeof(picture));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            
            ret = hld_decv_mp_capture_frame(info, &picture);
            if(ret != 0)
            {
                break;
            }
            
            ret = copy_to_user((void __user *)arg, &picture, sizeof(picture));
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }

        case VDECIO_MP_SET_DEC_FRM_TYPE:
        {
            unsigned long type = arg;

            ret = hld_decv_mp_set_dec_frm_type(info, (UINT32)type);
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            
            break;
        }

        case VDECIO_MP_DYNAMIC_FRAME_ALLOC:
        {
            unsigned long enable = arg;

            ret = hld_decv_mp_ioctl(info, VDEC_DYNAMIC_FRAME_ALLOC, (UINT32)enable);
            if(ret != 0)
            {
                ret = -EFAULT;
            }

            break;
        }
            
        default:
            ret = video_ioctl_internal(info, cmd, arg);
            break;
    }

    return ret;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
static long ali_video_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int ali_video_ioctl(struct inode *node,struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    struct ali_video_info *info = (struct ali_video_info *)file->private_data;
    int ret = 0;

    down(&info->sem);

    if(info->open_count <= 0)
    {
        VDEC_PRINTF("video is not opened\n");
        goto EXIT;
    }
    switch(cmd) 
    {
        case VDECIO_START:
        {
            ret = hld_decv_start(info);
            if(ret == 0)
            {
                if(info->status == ALI_VIDEO_IDLE)
                {
                    info->status = ALI_VIDEO_WORKING;
                }
            }
            break;
        }

        case VDECIO_STOP:
        {
            struct vdec_stop_param stop_param;
            
            ret = copy_from_user(&stop_param, (void __user *)arg, sizeof(stop_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            
            ret = hld_decv_stop(info, &stop_param);
            if(ret == 0)
            {
                if(info->status == ALI_VIDEO_WORKING || info->status == ALI_VIDEO_PAUSED)
                {
                    info->status = ALI_VIDEO_IDLE;
                }
            }
            break;
        }

        case VDECIO_PAUSE:
            ret = hld_decv_pause(info);
            break;

        case VDECIO_RESUME:
            ret = hld_decv_resume(info);
            break;

        case VDECIO_STEP:
            ret = hld_decv_step(info);
            break;

        case VDECIO_SET_SYNC_MODE:
        {
            struct vdec_sync_param sync_param;

            ret = copy_from_user(&sync_param, (void __user *)arg, sizeof(sync_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            
            ret = hld_decv_set_sync_mode(info, &sync_param);
            break;
        }

        case VDECIO_SET_PLAY_MODE:
        {
            struct vdec_playback_param playback_param;
            
            ret = copy_from_user(&playback_param, (void __user *)arg, sizeof(playback_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            ret = hld_decv_set_play_mode(info, &playback_param);
            break;
        }

        case VDECIO_SET_PVR_PARAM:
        {
            struct vdec_pvr_param pvr_param;

            ret = copy_from_user(&pvr_param, (void __user *)arg, sizeof(pvr_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            ret = hld_decv_set_pvr_param(info, &pvr_param);
            break;
        }

        case VDECIO_SELECT_DECODER:
        {
            struct vdec_codec_param codec_param;
            
            ret = copy_from_user(&codec_param, (void __user *)arg, sizeof(codec_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            ret = hld_decv_select_decoder(info, &codec_param);
            break;
        }

        case VDECIO_GET_CUR_DECODER:
        {
            enum vdec_type type = VDEC_MPEG;

            type = hld_decv_get_cur_decoder(info);
            ret = copy_to_user((void __user *)arg, &type, sizeof(type));
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }

        case VDECIO_SET_OUTPUT_MODE:
        {
            struct vdec_output_param output_param;
            
            ret = copy_from_user(&output_param, (void __user *)arg, sizeof(output_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            ret = hld_decv_set_output(info, &output_param);
            if(ret != 0)
            {
                break;
            }
            
            ret = copy_to_user((void __user *)arg, &output_param, sizeof(output_param));
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }

        case VDECIO_GET_STATUS:
        {
            struct vdec_status_info vdec_stat;
            
            memset(&vdec_stat, 0, sizeof(vdec_stat));
            
            ret = hld_decv_ioctl(info, VDEC_IO_GET_STATUS, (UINT32)&vdec_stat);
            if(ret != 0)
            {
                break;
            }
            
            ret = copy_to_user((void __user *)arg, &vdec_stat, sizeof(vdec_stat));
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }

        case VDECIO_FIRST_I_FREERUN:
        {
            int first_i_freerun = (int)arg;

            ret = hld_decv_ioctl(info, VDEC_IO_FIRST_I_FREERUN, first_i_freerun);
            VDEC_PRINTF("video set first i free run %d\n", first_i_freerun);
            break;
        }

        case VDECIO_SET_SYNC_DELAY:
        {
            unsigned long sync_delay = arg;

            ret = hld_decv_ioctl(info, VDEC_IO_SET_SYNC_DELAY, sync_delay);
            VDEC_PRINTF("video set sync delay %lu\n", sync_delay);
            break;
        }

        case VDECIO_CONTINUE_ON_ERROR:
        {
            unsigned long continue_on_err = arg;

            ret = hld_decv_ioctl(info, VDEC_IO_CONTINUE_ON_ERROR, continue_on_err);
            VDEC_PRINTF("video set continue on error %lu\n", continue_on_err);
            break;
        }

        case VDECIO_SET_OUTPUT_RECT:
        {
            struct vdec_display_rect display_rect;
            struct VDecPIPInfo pip_info;

            memset(&pip_info, 0, sizeof(pip_info));
            
            ret = copy_from_user(&display_rect, (void __user *)arg, sizeof(display_rect));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            pip_info.adv_setting.switch_mode = 1;
            pip_info.src_rect.uStartX = display_rect.src_x;
            pip_info.src_rect.uStartY = display_rect.src_y;
            pip_info.src_rect.uWidth  = display_rect.src_w;
            pip_info.src_rect.uHeight = display_rect.src_h;
            pip_info.dst_rect.uStartX = display_rect.dst_x;
            pip_info.dst_rect.uStartY = display_rect.dst_y;
            pip_info.dst_rect.uWidth  = display_rect.dst_w;
            pip_info.dst_rect.uHeight = display_rect.dst_h;
            ret = hld_decv_ioctl(info, VDEC_IO_SET_OUTPUT_RECT, (unsigned long)&pip_info);
            
            VDEC_PRINTF("video set output rect <%ld %ld %ld %ld> => <%ld %ld %ld %ld>\n", 
                display_rect.src_x, display_rect.src_y, display_rect.src_w, display_rect.src_h,
                display_rect.dst_x, display_rect.dst_y, display_rect.dst_w, display_rect.dst_h);
            break;
        }

        case VDECIO_CAPTURE_DISPLAYING_FRAME:
        {
            struct vdec_picture picture;
            
            ret = copy_from_user(&picture, (void __user *)arg, sizeof(picture));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            ret = hld_decv_ioctl(info, VDEC_IO_CAPTURE_DISPLAYING_FRAME, (UINT32)&picture);
            if(ret != 0)
            {
                break;
            }
            
            ret = copy_to_user((void __user *)arg, &picture, sizeof(picture));
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }

        case VDECIO_FILL_FRAME:
        {
            struct vdec_yuv_color yuv_color;
            struct YCbCrColor color;

            ret = copy_from_user(&yuv_color, (void __user *)arg, sizeof(yuv_color));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }

            color.uY  = yuv_color.y;
            color.uCb = yuv_color.u;
            color.uCr = yuv_color.v;
            ret = hld_decv_ioctl(info, VDEC_IO_FILL_FRM, (unsigned long)&color);
            VDEC_PRINTF("video fill color <0x%x 0x%x 0x%x>\n", yuv_color.y, yuv_color.u, yuv_color.v);
            break;
        }

        case VDECIO_DRAW_COLOR_BAR:
        {
            unsigned long color_bar_addr = arg;
            
            ret = hld_decv_ioctl(info, VDEC_IO_COLORBAR, __VMTSALI(color_bar_addr));
            VDEC_PRINTF("video draw color bar %lu\n", color_bar_addr);
            break;
        }

        case VDECIO_SET_DMA_CHANNEL:
        {
            unsigned char channel_num = (unsigned char)arg;
            
            ret = hld_decv_ioctl(info, VDEC_SET_DMA_CHANNEL, channel_num);
            VDEC_PRINTF("video set dma channel %u\n", channel_num);
            break;
        }

        case VDECIO_DTVCC_PARSING_ENABLE:
        {
            int enable = (int)arg;
            
            ret = hld_decv_ioctl(info, VDEC_DTVCC_PARSING_ENABLE, enable);
            VDEC_PRINTF("video set dtvcc parsing enable %d\n", enable);
            break;
        }

        case VDECIO_SAR_ENABLE:
        {
            int enable = (int)arg;
            
            ret = hld_decv_ioctl(info, VDEC_IO_SAR_ENABLE, enable);
            VDEC_PRINTF("video set sar enable %d\n", enable);
            break;
        }

        case VDECIO_SET_DEC_FRM_TYPE:
        {
            unsigned long type = arg;
            
            ret = hld_decv_ioctl(info, VDEC_IO_SET_DEC_FRM_TYPE, type);
            VDEC_PRINTF("video set dec frm type %lu\n", type);
            break;
        }

        case VDECIO_SET_SIMPLE_SYNC:
        {
            int enable = (int)arg;
            
            ret = hld_decv_ioctl(info, VDEC_IO_SET_SIMPLE_SYNC, enable);
            VDEC_PRINTF("video set simple sync %d\n", enable);
            break;
        }

        case VDECIO_SET_TRICK_MODE:
        {
            struct vdec_playback_param playback_param;
            
            ret = copy_from_user(&playback_param, (void __user *)arg, sizeof(playback_param));
            if(ret != 0)
            {
                ret = -EFAULT;
                break;
            }
            
            ret = hld_decv_ioctl(info, VDEC_IO_SET_TRICK_MODE, (UINT32)&playback_param);
            VDEC_PRINTF("video set trick mode %d %d\n", playback_param.direction, playback_param.rate);
            break;
        }

        case VDECIO_VBV_BUFFER_OVERFLOW_RESET:
        {
            int reset = (int)arg;
            
            ret = hld_decv_ioctl(info, VDEC_VBV_BUFFER_OVERFLOW_RESET, reset);
            VDEC_PRINTF("video set vbv overflow reset %d\n", reset);
            break;
        }

        case VDECIO_GET_MEM_INFO:
        {
            struct ali_video_mem_info mem_info;
            
            mem_info.mem_size  = info->mem_size;
            mem_info.mem_start = (void *)(info->mem_addr & 0x1FFFFFFF);
            mem_info.priv_mem_size  = info->priv_mem_size;
            mem_info.priv_mem_start = (void *)(info->priv_mem_addr & 0x1FFFFFFF);
            mem_info.mp_mem_size  = info->mp_mem_size;
            mem_info.mp_mem_start = (void *)(info->mp_mem_addr & 0x1FFFFFFF);
            
            ret = copy_to_user((void __user *)arg, (void *)&mem_info, sizeof(mem_info));
            if(ret != 0)
            {
                ret = -EFAULT;
            }
            break;
        }

        case VDECIO_SET_SOCK_PORT_ID:
        {
            info->dst_pid = (int)arg;
            hld_decv_rpc_init(info);
            break;
        }
        
        case VDECIO_REG_CALLBACK:
        {
            struct vdec_io_reg_callback_para cb_param;
            memset(&cb_param, 0, sizeof(cb_param));
            cb_param.eCBType = (unsigned long)arg;
            cb_param.pCB = info->call_back.pcb_vdec_user_data_parsed;
            ret = hld_decv_ioctl(info, VDEC_IO_REG_CALLBACK, (UINT32)(&cb_param));
            VDEC_PRINTF("video register callback %lu\n", arg);
            break;
        }

        case VDECIO_UNREG_CALLBACK:
        {
            struct vdec_io_reg_callback_para cb_param;
            memset(&cb_param, 0, sizeof(cb_param));
            cb_param.eCBType = (unsigned long)arg;
            cb_param.pCB = NULL;
            ret = hld_decv_ioctl(info, VDEC_IO_REG_CALLBACK, (UINT32)(&cb_param));     
            VDEC_PRINTF("video unregister callback %lu\n", arg);
            break;
        }
            
        default:
            ret = ali_video_mp_ioctl(info, cmd, arg);
            break;
    }
EXIT:
    up(&info->sem);
    
    return ret;
}

int ali_video_suspend(struct device *dev, pm_message_t state)
{
    struct ali_video_info *info = dev_get_drvdata(dev);

    if(info->status == ALI_VIDEO_WORKING) {
        //vdec_stop(info->cur_dev, 0, 0);
        hld_decv_rpc_suspend(info);
    }

    VDEC_PRINTF("video %u suspend %u\n", info->vdec_type, info->status);
    
    return 0;
}

int ali_video_resume(struct device *dev)
{
    struct ali_video_info *info = dev_get_drvdata(dev);

    if(info->status == ALI_VIDEO_WORKING) {
        //vdec_start(info->cur_dev);
        hld_decv_rpc_resume(info);
    }

    VDEC_PRINTF("video %u resume %u\n", info->vdec_type, info->status);
    
    return 0;
}

static int ali_video_open(struct inode *inode, struct file *file)
{
    struct ali_video_info *info;
    int i = 0;
  
    /* Get the per-device structure that contains this cdev */
    info = container_of(inode->i_cdev, struct ali_video_info, cdev);

    down(&info->sem);

    /* Easy access to sbm_devp from rest of the entry points */
    file->private_data = info;

    if(info->open_count == 0) {
        /* Initialize some fields */
        for(i = 0; i < MAX_VIDEO_RPC_ARG_NUM; i++) {
            info->rpc_arg[i] = kmalloc(MAX_VIDEO_RPC_ARG_SIZE, GFP_KERNEL);
            if(info->rpc_arg[i] == NULL) {
                up(&info->sem);
                VDEC_PRINTF("video malloc rpc arg buf fail\n");
                return -1;
            }
        }

        //vdec_open(info->cur_dev);
        info->status = ALI_VIDEO_IDLE;
    }

    info->open_count++;

    up(&info->sem);
    
    return 0;
}

static int ali_video_release(struct inode *inode, struct file *file)
{
    struct ali_video_info *info = (struct ali_video_info *)file->private_data;
    int i = 0;

    down(&info->sem);

    info->open_count--;

    if(info->open_count == 0) {
        /* Release some fields */
        for(i = 0;i < MAX_VIDEO_RPC_ARG_NUM;i++) {
            if(info->rpc_arg[i] != NULL) {
                kfree((void*)info->rpc_arg[i]);
                info->rpc_arg[i] = NULL;
            }
        }
    
        //vdec_close(info->cur_dev);
    }

    up(&info->sem);
    
    return 0;
}

void ali_decv_rpc_release(void)
{
    down(&ali_video_priv->sem);
    
    //hld_decv_rpc_release();
    hld_decv_rpc_release(ali_video_priv, 0);

    up(&ali_video_priv->sem);           
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations ali_video_fops = {
    .owner    =   THIS_MODULE,           /* Owner */
    .open     =   ali_video_open,        /* Open method */
    .release  =   ali_video_release,     /* Release method */
    .read     =   ali_video_read,        /* Read method */
    .write    =   ali_video_write,       /* Write method */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	.unlocked_ioctl = ali_video_ioctl,
#else
	.ioctl = ali_video_ioctl,
#endif	
};

#define DEVICE_NAME                      "ali_video"
#define DEVICE_NUM                       1

static dev_t ali_video_dev_t;            /* Allotted device number */
static struct class *ali_video_class;    /* Tie with the device model */
static struct device *ali_video_device;

static int __init ali_video_init(void)
{
    int ret;
    #if 0
    video_mem_attr_t *p_video_mem_attr = (video_mem_attr_t *)request_attr(VIDEO_MEM);
    mp_mem_attr_t *p_mp_mem_attr = (mp_mem_attr_t *)request_attr(MP_MEM);
#endif
    /* Allocate memory for the per-device structure */
    ali_video_priv = kmalloc(sizeof(struct ali_video_info), GFP_KERNEL);
    if (!ali_video_priv) {
        VDEC_PRINTF("Video bad Kmalloc\n"); return -ENOMEM;
    }
    memset(ali_video_priv, 0, sizeof(struct ali_video_info));
    /* init current device to MPEG2 decoder */
    ali_video_priv->cur_dev = (struct vdec_device *)hld_dev_get_by_name("DECV_S3601_0");
    ali_video_priv->mem_addr = __G_ALI_MM_VIDEO_START_ADDR;
    ali_video_priv->mem_size = __G_ALI_MM_VIDEO_SIZE;
    ali_video_priv->priv_mem_addr = 0;
    ali_video_priv->priv_mem_size = 0;
    ali_video_priv->mp_mem_addr = __G_ALI_MM_APE_MEM_START_ADDR;
    ali_video_priv->mp_mem_size = __G_ALI_MM_APE_MEM_SIZE;
    ali_video_priv->capture_mem_addr = __G_ALI_MM_STILL_FRAME_START_ADDR;
    ali_video_priv->capture_mem_size = __G_ALI_MM_STILL_FRAME_SIZE;
    ali_video_priv->tmp_mem_addr = __G_ALI_MM_APE_MEM_START_ADDR;//kmalloc(TMP_BUF_SIZE, GFP_KERNEL);
    ali_video_priv->tmp_mem_size = TMP_BUF_SIZE;
  
    /* Request dynamic allocation of a device major number */
    if (alloc_chrdev_region(&ali_video_dev_t, 0,
                            DEVICE_NUM, DEVICE_NAME) < 0) {
        VDEC_PRINTF(KERN_DEBUG "Video can't register device\n"); 
        goto fail;
    }

    /* Populate sysfs entries */
    ali_video_class = class_create(THIS_MODULE, DEVICE_NAME);
    if(ali_video_class == NULL){
        VDEC_PRINTF("Video create class fail\n");
        goto fail;
    } 
    ali_video_class->suspend = ali_video_suspend;
    ali_video_class->resume = ali_video_resume;

    /* Connect the file operations with the cdev */
    cdev_init(&ali_video_priv->cdev, &ali_video_fops);
    ali_video_priv->cdev.owner = THIS_MODULE;
    kobject_set_name(&(ali_video_priv->cdev.kobj), "%s", "ali_video");

    /* Connect the major/minor number to the cdev */
    ret = cdev_add(&ali_video_priv->cdev, ali_video_dev_t, 1);
    if (ret) {
        VDEC_PRINTF("Video bad cdev\n");
        goto fail;
    }
    
    ali_video_device = device_create(ali_video_class, NULL, MKDEV(MAJOR(ali_video_dev_t), 0), 
                                     ali_video_priv, "ali_video%d", 0);
    if(ali_video_device == NULL){
        VDEC_PRINTF("Video create device fail\n");
        goto fail;
    }  
    
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
	init_MUTEX(&ali_video_priv->sem);
#else
	sema_init(&ali_video_priv->sem, 1);
#endif

    ali_video_dbg_on = 0;
    ali_video_ape_dbg_on = 0;

    return 0;

fail:
    if(ali_video_dev_t != 0)
        unregister_chrdev_region(ali_video_dev_t, DEVICE_NUM);
    if(ali_video_device != NULL)
        device_del(ali_video_device);
    if(ali_video_class != NULL)
        class_destroy(ali_video_class);
    /*if(ali_video_priv->tmp_mem_addr)
        kfree((void*)ali_video_priv->tmp_mem_addr);*/
    if(ali_video_priv != NULL)
        kfree(ali_video_priv);

    VDEC_PRINTF("video init fail\n");
    return -1;
}

/* Driver Exit */
void __exit ali_video_exit(void)
{  
    /* Release the major number */
    unregister_chrdev_region(ali_video_dev_t, DEVICE_NUM);

    device_destroy(ali_video_class, MKDEV(MAJOR(ali_video_dev_t), 0));
    /* Remove the cdev */
    cdev_del(&ali_video_priv->cdev);    
    /* Destroy ali_video_class */
    class_destroy(ali_video_class);
    
    /*if(ali_video_priv->tmp_mem_addr) {
        kfree((void*)ali_video_priv->tmp_mem_addr);
    }*/

    kfree(ali_video_priv);

    ali_video_dev_t = 0;
    ali_video_device = NULL;
    ali_video_class = NULL;
    ali_video_priv = NULL;
    
    return;
}

module_init(ali_video_init);
module_exit(ali_video_exit);
 
MODULE_AUTHOR("ALi (Shanghai) Corporation");
MODULE_DESCRIPTION("ali video player driver");
MODULE_LICENSE("GPL");

