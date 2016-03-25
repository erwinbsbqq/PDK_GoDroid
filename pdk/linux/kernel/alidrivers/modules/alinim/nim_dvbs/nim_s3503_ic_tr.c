/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_s3503_ic_01.c
*
*    Description:  s3503 ic layer
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_s3503.h"

#define SYM_6500 6500
#define SYM_33000 33000
#define SYM_16000 16000
#define SYM_12000 12000
#define SYM_4000   4000
#define CELL_ID_3 3
#define CELL_ID_11 11
#define CELL_ID_12 12
#define CELL_ID_13 13
#define CELL_ID_14 14


static UINT8 last_value = 0;


static const UINT8 map_beta_active_buf[32] =
{
    0x00,   //                           //index 0, do not use
    0x01,   // 1/4 of QPSK              //1
    0x01,   // 1/3                      //2
    0x01,   // 2/5                      //3
    0x01,   // 1/2                      //4
    0x01,   // 3/5                      //5
    0x01,   // 2/3                      //6
    0x01,   // 3/4                      //7
    0x01,   // 4/5                      //8
    0x01,   // 5/6                      //9
    0x01,   // 8/9                      //a
    0x01,   // 9/10                     //b
    0x01,   // 3/5 of 8PSK              //c
    0x01,   // 2/3                      //d
    0x01,   // 3/4                      //e
    0x01,   // 5/6                      //f
    0x01,   // 8/9                      //10
    0x01,   // 9/10                     //11
    0x01,   // 2/3 of 16APSK            //12
    0x01,   // 3/4                      //13
    0x01,   // 4/5                      //14
    0x01,   // 5/6                      //15
    0x01,   // 8/9                      //16
    0x01,   // 9/10                     //17
    0x01,   // for 32 APSK, dont use
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};

static const UINT8 map_beta_buf_llr_shift_off[32] =
{
    188,   //  205,        // index 0, do not use
    188,   //  205,        // 1/4 of QPSK    //1
    190,   //  230,        // 1/3                      //2
    205,   //  205,        // 2/5                      //3
    205,   //  205,        // 1/2                      //4
    180,   //  180,        // 3/5                      //5
    180,   //  180,        // 2/3                      //6
    180,   //  180,        // 3/4                      //7
    155,   //  180,        // 4/5                      //8
    168,   //  180,        // 5/6                      //9
    150,   //  155,        // 8/9                      //a
    150,   //  155,        // 9/10                     //b
    215,   //  180,        // 3/5 of 8PSK              //c
    185,   //  180,        // 2/3                      //d
    145,   //  180,        // 3/4                      //e
    145,   //  155,        // 5/6                      //f
    145,   //  155,        // 8/9                      //10
    140,   //  155,        // 9/10                     //11
    185,   //  205,        // 2/3 of 16APSK            //12
    165,   //  180,        // 3/4                      //13
    145,   //  180,        // 4/5                      //14
    130,   //  155,        // 5/6                      //15
    145,   //  155,        // 8/9                      //16
    130,   //  155,        // 9/10                     //17
    145,    //3/4-    for 32 APSK,
    120,     // 4/5
    155,      //5/6
    140,   // 8/9
    125,   // 9/10
    155,
    155,
    155
};

static const UINT8 map_beta_buf_llr_shift_on[32] =
{
    188,   //  205,        // index 0, do not use
    188,   //  205,        // 1/4 of QPSK    //1
    190,   //  230,        // 1/3                      //2
    205,   //  205,        // 2/5                      //3
    205,   //  205,        // 1/2                      //4
    180,   //  180,        // 3/5                      //5
    180,   //  180,        // 2/3                      //6
    180,   //  180,        // 3/4                      //7
    155,   //  180,        // 4/5                      //8
    168,   //  180,        // 5/6                      //9
    150,   //  155,        // 8/9                      //a
    150,   //  155,        // 9/10                     //b
    215,   //  180,        // 3/5 of 8PSK              //c
    185,   //  180,        // 2/3                      //d
    140,   //  180,        // 3/4                      //e
    230,   //  155,        // 5/6                      //f
    160,   //  155,        // 8/9                      //10
    155,   //  155,        // 9/10                     //11
    185,   //  205,        // 2/3 of 16APSK            //12
    155,   //  180,        // 3/4                      //13
    140,   //  180,        // 4/5                      //14
    160,   //  155,        // 5/6                      //15
    160,   //  155,        // 8/9                      //16
    160,   //  155,        // 9/10                     //17
    145,    //3/4-    for 32 APSK,
    120,     // 4/5
    145,      //5/6
    135,   // 8/9
    130,   // 9/10
    135,
    155,
    155
};

static const UINT16 demap_noise[32] =
{
    0x00,       // index 0, do not use
    0x16b,      // 1/4 of QPSK              //1
    0x1d5,      // 1/3                      //2
    0x246,      // 2/5                      //3
    0x311,      // 1/2                      //4
    0x413,      // 3/5                      //5
    0x4fa,      // 2/3                      //6
    0x62b,      // 3/4                      //7
    0x729,      // 4/5                      //8
    0x80c,      // 5/6                      //9
    0xa2a,      // 8/9                      //a
    0xab2,      // 9/10                     //b
    0x8a9,      // 3/5 of 8PSK              //c
    0xb31,      // 2/3                      //d
    0xf1d,   // 3/4                         //e
    0x1501,     // 5/6                      //f
    0x1ca5,       // 8/9                    //10
    0x1e91,       // 9/10                   //11
    0x133b,       // 2/3 of 16APSK          //12
    0x199a,       // 3/4                    //13
    0x1f08,       // 4/5                    //14
    0x234f,       // 5/6                    //15
    0x2fa1,       // 8/9                    //16
    0x3291,        // 9/10                  //17
    0x00,        // for 32 APSK, dont use
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



static const UINT32 agc_table[256] =
{
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   3,   4,   5, \
    6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21, \
    22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37, \
    38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  52,  55,  57, \
    60,  62,  65,  67,  70,  72,  75,  77,  78,  79,  80,  82,  83,  84,  85,  87, \
    88,  89,  90,  92,  93,  94,  95,  97,  98,  99, 101, 102, 103, 104, 105, 106, \
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, \
    123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, \
    139, 140, 141, 142, 144, 145, 146, 147, 149, 150, 151, 152, 154, 155, 156, 157, \
    159, 160, 161, 162, 164, 165, 166, 167, 169, 170, 171, 173, 175, 176, 178, 180, \
    182, 184, 187, 189, 192, 194, 197, 199, 202, 204, 207, 209, 212, 215, 218, 222, \
    225, 228, 232, 235, 238, 242, 245, 248, 252, 255, 258, 262, 265, 268, 272, 275, \
    278, 282, 285, 288, 292, 295, 298, 302, 305, 308, 312, 317, 322, 327, 332, 337, \
    342, 347, 352, 357, 362, 367, 372, 377, 382, 387, 392, 397, 402, 407, 412, 417, \
    422, 427, 432, 437, 442, 447, 452, 457, 462, 467, 472, 477, 482, 487, 492, 493, \
    494, 495, 496, 497, 498, 499, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, \
};






static const UINT16 s2_awgn_noise[28] = // 10bits
{
    //
    0x1ff,//"QPSK 1/4", // 1 <=0.4dB
    0x1ff,//"QPSK 1/3", // 2
    0x1ff,//"QPSK 2/5", // 3
    0x005,//0x1ed,//"QPSK 1/2", // 4 1.7dB
    0x1bc,//0x1c3,//"QPSK 3/5", // 5 2.6dB
    0x1ff,//"QPSK 2/3", // 6 3.6dB
    0x1ff,//"QPSK 3/4", // 7 4.5dB
    0x1ff,//"QPSK 4/5", // 8 5.2dB
    0x1ff,//"QPSK 5/6", // 9 5.6dB
    0x1ff,//"QPSK 8/9", // 10
    0x1ff,//"QPSK 9/10", // 11 I set to 0x1ff because the value is small to 0x115 at the AWGN, but at the phase noise
    // when the CR is locked, the EST_NOISE is high to 0x15a, and so that the widerloop cannot be openecd.
    // and then CR will be unlocked.
    // this setting does not effect the AWGN performance.
    // symbol rate 6500below:   up
    0xcd,  //"8PSK 3/5", 6.5dB
    0xb9, //"8PSK 2/3", // 13 7.5dB
    0xa5,  //"8PSK 3/4", // 14 8.6dB
    0x90, //"8PSK 5/6", // 15 9.7dB
    0x83,//"8PSK 8/9", // 16 11.1dB phase noise
    0x83,//"8PSK 9/10", // 17 11.1dB(Phase noise)
    0x5f,//"16APSK 2/3", // 18 11.5dB
    0x59,//"16APSK 3/4", // 19 12.0dB
    0x56,//"16APSK 4/5", // 20 12.3dB0
    0x57,//"16APSK 5/6", // 21 12.2dB
    0x47,//"16APSK 8/9", // 22 13.6dB(phase noise)
    0x46,//"16APSK 9/10", // 23 13.7dB(phase noise)
    0x35,//"32APSK 3/4", // 24 14.3dB(phase noise)
    0x31,//"32APSK 4/5", // 25 15.3dB(phase noise)
    0x30,//"32APSK 5/6", // 26 15.5dB(phase noise)
    0x26,//"32APSK 8/9", // 27 16.70dB
    0x22//"32APSK 9/10", // 28 17.3dB
};

static const UINT16 S2_AWGN_NOISE_EXTRA[28] = // 10bits
{
    // 6.5<=symbol_rate<=33M
    0x1ff,//"QPSK 1/4", // 1 <=0.4dB
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0,//x190,//"QPSK 1/2",3.0dB awgn for wider loop enable
    0x1ff,//"QPSK 3/5", // 5 same to QPSK 8/9
    0x1ff,//"QPSK 2/3", // 6
    0x1ff,//"QPSK 3/4", // 7
    0x1ff,//"QPSK 4/5", // 8
    0x1ff,//"QPSK 5/6", // 9
    0x1ff,//"QPSK 8/9", // 10
    0x1ff,//"QPSK 9/10", // 11 I set to 0x1ff because the value is small to 0x115 at the AWGN, but at the phase noise
    // when the CR is locked, the EST_NOISE is high to 0x15a, and so that the widerloop cannot be openecd.
    // and then CR will be unlocked.
    // this setting does not effect the AWGN performance.
    // symbol rate 6500below:   up
    0xc0, //"8PSK 3/5", // 12 6.5dB
    0xb0, //"8PSK 2/3", // 13 7.5dB
    0x9e, //"8PSK 3/4", // 14 8.6dB
    0x8c, //"8PSK 5/6", // 15 9.7dB
    0x7a,//"8PSK 8/9", // 16 11.1dB phase noise
    0x7a,//"8PSK 9/10", // 17 11.1dB(Phase noise)
    0x5b,//"16APSK 2/3", // 18 11.5dB
    0x55,//"16APSK 3/4", // 19 12.0dB
    0x54,//"16APSK 4/5", // 20 12.3dB0
    0x54,//"16APSK 5/6", // 21 12.2dB
    0x47,//"16APSK 8/9", // 22 13.6dB(phase noise)
    0x43,//"16APSK 9/10", // 23 14.0 awgn    //(dont use)->13.7dB(phase noise)
    0x33,//"32APSK 3/4", // 24 14.3dB(phase noise)
    0x2e,//"32APSK 4/5", // 25 15.3dB(phase noise)
    0x2d,//"32APSK 5/6", // 26 15.5dB(phase noise)
    0x25,//"32APSK 8/9", // 27 16.70dB
    0x23//"32APSK 9/10", // 28 17.3dB
};

static const UINT8 s2_awgn_coef_set[28] = //3    //3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x2,//0x1,//"QPSK 1/2", // 4
    0x6,//"QPSK 3/5", // 5
    0x7,//"QPSK 2/3", // 6
    0x0,//"QPSK 3/4", // 7
    0x2,//"QPSK 4/5", // 8  0x0////
    0,//"QPSK 5/6", // 9
    0,//"QPSK 8/9", // 10
    0x1,//"QPSK 9/10", // 11
    0x0,//"8PSK 3/5", // 12
    0x3,//"8PSK 2/3", // 13
    0x3,//"8PSK 3/4", // 14
    0x3,//"8PSK 5/6", // 15
    0x2,//"8PSK 8/9", // 16
    0x0,//"8PSK 9/10", // 17
    0x2,//"16APSK 2/3", // 18
    0x0,//"16APSK 3/4", // 19
    0x1,//"16APSK 4/5", // 20
    0x2,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0x0,//"16APSK 9/10", // 23
    0,//"32APSK 3/4", // 24
    0x0,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0x2,//"32APSK 8/9", // 27
    0x2//"32APSK 9/10", // 28
};
static const UINT8 s2_awgn_coef_set_extra[28] = //3    //3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x2,//0x1,20130703//"QPSK 1/2", // 4
    0x1,//"QPSK 3/5", // 5
    0x6,//"QPSK 2/3", // 6
    0x6,//"QPSK 3/4", // 7
    0x1,//"QPSK 4/5", // 8
    0x6,//"QPSK 5/6", // 9
    0x6,//"QPSK 8/9", // 10
    0x6,//"QPSK 9/10", // 11
    0x1,//"8PSK 3/5", // 12
    0x3,//"8PSK 2/3", // 13
    0x3,//"8PSK 3/4", // 14
    0x3,//"8PSK 5/6", // 15
    0x2,//"8PSK 8/9", // 16
    0x0,//"8PSK 9/10", // 17
    0x4,//"16APSK 2/3", // 18
    0x0,//"16APSK 3/4", // 19
    0x2,//"16APSK 4/5", // 20
    0x2,//"16APSK 5/6", // 21
    0x1,//"16APSK 8/9", // 22
    0x1,//"16APSK 9/10", // 23
    0,//"32APSK 3/4", // 24
    0x0,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0x0,//"32APSK 8/9", // 27
    0x5//"32APSK 9/10", // 28
};

static const UINT8 s2_pon_irs_delta[28] = //4    //3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x0,//"QPSK 1/2", // 4
    0x7,//"QPSK 3/5", // 5
    0x1,//"QPSK 2/3", // 6
    0,//"QPSK 3/4", // 7
    0,//"QPSK 4/5", // 8
    0,//"QPSK 5/6", // 9
    0,//"QPSK 8/9", // 10
    0,//"QPSK 9/10", // 11
    0x7,//"8PSK 3/5", // 12
    0x0,//"8PSK 2/3", // 13
    0x0,//"8PSK 3/4", // 14
    0x0,//"8PSK 5/6", // 15
    0x7,//"8PSK 8/9", // 16
    0x0,//"8PSK 9/10", // 17
    0x0,//"16APSK 2/3", // 18
    0x0,//"16APSK 3/4", // 19
    0x0,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0,//"16APSK 9/10", // 23
    0x7,//"32APSK 3/4", // 24
    0x7,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0x0,//"32APSK 8/9", // 27
    0x1//"32APSK 9/10", // 28
};
static const UINT8 s2_pon_irs_delta_extra[28] = //4    //3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x6,//0x0,//"QPSK 1/2", // 4
    0x1,//"QPSK 3/5", // 5
    0,//"QPSK 2/3", // 6
    0,//"QPSK 3/4", // 7
    0,//"QPSK 4/5", // 8
    0,//"QPSK 5/6", // 9
    0,//"QPSK 8/9", // 10
    0,//"QPSK 9/10", // 11
    0x7,//"8PSK 3/5", // 12
    0x0,//"8PSK 2/3", // 13
    0x0,//"8PSK 3/4", // 14
    0x7,//"8PSK 5/6", // 15
    0x7,//"8PSK 8/9", // 16
    0x0,//"8PSK 9/10", // 17
    0x7,//"16APSK 2/3", // 18
    0x7,//"16APSK 3/4", // 19
    0x7,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0,//"16APSK 9/10", // 23
    0x7,//"32APSK 3/4", // 24
    0x7,//"32APSK 4/5", // 25
    0x7,//"32APSK 5/6", // 26
    0x7,//"32APSK 8/9", // 27
    0x7//"32APSK 9/10", // 28
};

static const UINT8 s2_pon_prs_delta[28] = // 3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x0,//"QPSK 1/2", // 4
    0x7,//"QPSK 3/5", // 5
    0x7,//"QPSK 2/3", // 6
    0,//"QPSK 3/4", // 7
    0,//"QPSK 4/5", // 8
    0,//"QPSK 5/6", // 9
    0,//"QPSK 8/9", // 10
    0,//"QPSK 9/10", // 11
    0x7,//"8PSK 3/5", // 12
    0x0,//"8PSK 2/3", // 13
    0x0,//"8PSK 3/4", // 14
    0x0,//"8PSK 5/6", // 15
    0x7,//"8PSK 8/9", // 16
    0x0,//"8PSK 9/10", // 17
    0x0,//"16APSK 2/3", // 18
    0x0,//"16APSK 3/4", // 19
    0x0,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0,//"16APSK 9/10", // 23
    0x7,//"32APSK 3/4", // 24
    0x7,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0x0,//"32APSK 8/9", // 27
    0x1//"32APSK 9/10", // 28
};
static const UINT8 s2_pon_prs_delta_extra[28] = // 3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x6,//0x0,//"QPSK 1/2", // 4
    0x7,//"QPSK 3/5", // 5
    0,//"QPSK 2/3", // 6
    0,//"QPSK 3/4", // 7
    0,//"QPSK 4/5", // 8
    0,//"QPSK 5/6", // 9
    0,//"QPSK 8/9", // 10
    0,//"QPSK 9/10", // 11
    0x7,//"8PSK 3/5", // 12
    0x0,//"8PSK 2/3", // 13
    0x0,//"8PSK 3/4", // 14
    0x7,//"8PSK 5/6", // 15
    0x7,//"8PSK 8/9", // 16
    0x0,//"8PSK 9/10", // 17
    0x7,//"16APSK 2/3", // 18
    0x7,//"16APSK 3/4", // 19
    0x7,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0x1,//"16APSK 8/9", // 22
    0x7,//"16APSK 9/10", // 23
    0x7,//"32APSK 3/4", // 24
    0x7,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0x0,//"32APSK 8/9", // 27
    0x1//"32APSK 9/10", // 28
};

static const UINT8 s2_loop_coef_set[28] = //3//3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x1,//"QPSK 1/2", // 4
    0x7,//"QPSK 3/5", // 5
    0x4,//"QPSK 2/3", // 6
    0x4,//"QPSK 3/4", // 7
    0x4,//"QPSK 4/5", // 8
    0x4,//"QPSK 5/6", // 9
    0x4,//"QPSK 8/9", // 10
    0x1,//"QPSK 9/10", // 11
    0x6,//"8PSK 3/5", // 12
    0x6,//"8PSK 2/3", // 13
    0x5,//"8PSK 3/4", // 14
    0x5,//"8PSK 5/6", // 15
    0x7,//"8PSK 8/9", // 16
    0x5,//"8PSK 9/10", // 17
    0,//"16APSK 2/3", // 18
    0,//"16APSK 3/4", // 19
    0x3,//"16APSK 4/5", // 20
    0x2,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0x4,//"16APSK 9/10", // 23
    0x0,//"32APSK 3/4", // 24
    0,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0,//"32APSK 8/9", // 27
    0//"32APSK 9/10", // 28
};
static const UINT8 s2_loop_coef_set_extra[28] = //3//3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x1,//"QPSK 1/2", // 4
    0x7,//"QPSK 3/5", // 5
    0x1,//"QPSK 2/3", // 6
    0x1,//"QPSK 3/4", // 7
    0x7,//"QPSK 4/5", // 8
    0x1,//"QPSK 5/6", // 9
    0x1,//"QPSK 8/9", // 10
    0x1,//"QPSK 9/10", // 11
    0x7,//"8PSK 3/5", // 12
    0x5,//"8PSK 2/3", // 13
    0x2,//"8PSK 3/4", // 14
    0x5,//"8PSK 5/6", // 15
    0x7,//"8PSK 8/9", // 16
    0x5,//"8PSK 9/10", // 17
    0x3,//"16APSK 2/3", // 18
    0,//"16APSK 3/4", // 19
    0,//"16APSK 4/5", // 20
    0x2,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0x1,//"16APSK 9/10", // 23
    0x0,//"32APSK 3/4", // 24
    0,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0,//"32APSK 8/9", // 27
    0x3//"32APSK 9/10", // 28
};

static const UINT8 s2_cup_ped_set[28] = //3//3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x4,//"QPSK 1/2", // 4
    0x2,//"QPSK 3/5", // 5
    0x4,//"QPSK 2/3", // 6
    0x4,//"QPSK 3/4", // 7
    0x4,//"QPSK 4/5", // 8
    0x4,//"QPSK 5/6", // 9
    0x4,//"QPSK 8/9", // 10
    0x2,//"QPSK 9/10", // 11
    0x0,//"8PSK 3/5", // 12
    0,//"8PSK 2/3", // 13
    0,//"8PSK 3/4", // 14
    0x0,//"8PSK 5/6", // 15
    1,//"8PSK 8/9", // 16
    0x2,//"8PSK 9/10", // 17
    0,//"16APSK 2/3", // 18
    0,//"16APSK 3/4", // 19
    0x2,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0,//"16APSK 9/10", // 23
    0x2,//"32APSK 3/4", // 24
    0,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0x0,//"32APSK 8/9", // 27
    0//"32APSK 9/10", // 28
};
static const UINT8 s2_cup_ped_set_extra[28] = //3//3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x2,//"QPSK 1/2", // 4
    0x4,//"QPSK 3/5", // 5
    0x2,//"QPSK 2/3", // 6
    0x2,//"QPSK 3/4", // 7
    0x4,//"QPSK 4/5", // 8
    0x2,//"QPSK 5/6", // 9
    0x2,//"QPSK 8/9", // 10
    0x2,//"QPSK 9/10", // 11
    0,//"8PSK 3/5", // 12
    0,//"8PSK 2/3", // 13
    0,//"8PSK 3/4", // 14
    0,//"8PSK 5/6", // 15
    1,//"8PSK 8/9", // 16
    0x2,//"8PSK 9/10", // 17
    0x0,//"16APSK 2/3", // 18
    0x2,//"16APSK 3/4", // 19
    0x2,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0,//"16APSK 9/10", // 23
    0x2,//"32APSK 3/4", // 24
    0,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0x0,//"32APSK 8/9", // 27
    0//"32APSK 9/10", // 28
};

static const UINT8 s2_avg_ped_set[28] = //3//3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x7,//"QPSK 1/2", // 4
    0x5,//"QPSK 3/5", // 5
    0x5,//"QPSK 2/3", // 6
    0x5,//"QPSK 3/4", // 7
    0x5,//"QPSK 4/5", // 8
    0x5,//"QPSK 5/6", // 9
    0x5,//"QPSK 8/9", // 10
    0x7,//"QPSK 9/10", // 11
    0,//"8PSK 3/5", // 12
    0x1,//"8PSK 2/3", // 13
    0x1,//"8PSK 3/4", // 14
    0x2,//"8PSK 5/6", // 15
    0x3,//"8PSK 8/9", // 16
    0x3,//"8PSK 9/10", // 17
    0,//"16APSK 2/3", // 18
    0x2,//"16APSK 3/4", // 19
    0,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0x2,//"16APSK 8/9", // 22
    0x2,//"16APSK 9/10", // 23
    0x5,//"32APSK 3/4", // 24
    0x5,//"32APSK 4/5", // 25
    0x5,//"32APSK 5/6", // 26
    0x5,//"32APSK 8/9", // 27
    0x5//"32APSK 9/10", // 28
};

static const UINT8 s2_avg_ped_set_extra[28] = //3//3bit
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0x7,//"QPSK 1/2", // 4
    0x5,//"QPSK 3/5", // 5
    0x5,//"QPSK 2/3", // 6
    0x5,//"QPSK 3/4", // 7
    0x6,//"QPSK 4/5", // 8
    0x5,//"QPSK 5/6", // 9
    0x5,//"QPSK 8/9", // 10
    0x5,//"QPSK 9/10", // 11
    0,//"8PSK 3/5", // 12
    0x1,//"8PSK 2/3", // 13
    0x1,//"8PSK 3/4", // 14
    0x6,//"8PSK 5/6", // 15
    0x3,//"8PSK 8/9", // 16
    0x3,//"8PSK 9/10", // 17
    0,//"16APSK 2/3", // 18
    0x2,//"16APSK 3/4", // 19
    0,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0x2,//"16APSK 8/9", // 22
    0x2,//"16APSK 9/10", // 23
    0x5,//"32APSK 3/4", // 24
    0x5,//"32APSK 4/5", // 25
    0x5,//"32APSK 5/6", // 26
    0x5,//"32APSK 8/9", // 27
    0x5//"32APSK 9/10", // 28
};


static const UINT8 s2_force_old_cr[28] =
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0,//"QPSK 1/2", // 4
    0,//"QPSK 3/5", // 5
    0x1,//"QPSK 2/3", // 6
    0,//"QPSK 3/4", // 7
    0,//"QPSK 4/5", // 8
    0,//"QPSK 5/6", // 9
    0,//"QPSK 8/9", // 10
    0,//"QPSK 9/10", // 11
    0,//"8PSK 3/5", // 12
    0,//"8PSK 2/3", // 13
    0,//"8PSK 3/4", // 14
    0,//"8PSK 5/6", // 15
    1,//"8PSK 8/9", // 16
    0,//"8PSK 9/10", // 17
    0,//"16APSK 2/3", // 18
    0,//"16APSK 3/4", // 19
    0,//"16APSK 4/5", // 20
    0,//"16APSK 5/6", // 21
    0,//"16APSK 8/9", // 22
    0,//"16APSK 9/10", // 23
    0,//"32APSK 3/4", // 24
    0,//"32APSK 4/5", // 25
    0,//"32APSK 5/6", // 26
    0,//"32APSK 8/9", // 27
    0//"32APSK 9/10", // 28
};
static const UINT8 s2_llr_shift[28] =
{
    0,//"QPSK 1/4", // 1
    0,//"QPSK 1/3", // 2
    0,//"QPSK 2/5", // 3
    0,//"QPSK 1/2", // 4
    0,//"QPSK 3/5", // 5
    0,//"QPSK 2/3", // 6
    0,//"QPSK 3/4", // 7
    0,//"QPSK 4/5", // 8
    0,//"QPSK 5/6", // 9
    0,//"QPSK 8/9", // 10
    0,//"QPSK 9/10", // 11
    1,//"8PSK 3/5", // 12
    1,//"8PSK 2/3", // 13
    1,//"8PSK 3/4", // 14
    1,//"8PSK 5/6", // 15
    1,//"8PSK 8/9", // 16
    1,//"8PSK 9/10", // 17
    1,//"16APSK 2/3", // 18
    1,//"16APSK 3/4", // 19
    1,//"16APSK 4/5", // 20
    1,//"16APSK 5/6", // 21
    1,//"16APSK 8/9", // 22
    1,//"16APSK 9/10", // 23
    1,//"32APSK 3/4", // 24
    1,//"32APSK 4/5", // 25
    1,//"32APSK 5/6", // 26
    1,//"32APSK 8/9", // 27
    1//"32APSK 9/10", // 28
};


static const UINT8 symbol_before_head[8] =
{
    //                -AWGN_COEF_SET
    1,                // 8psk 3/5;
    1,                // 8psk 2/3;
    1,                //
    1,                //
    0,                // for DVBS
    0,                // dont use
    1,                // dotn use
    0                 //
};
static const UINT8 head_ped_gain[8] =
{
    //               -AWGN_COEF_SET
    0x1,              //  8psk 3/5;
    0x1,              //    8psk 2/3;
    0x1,              //    010
    0x1,              //    011
    0,                //  for DVBS
    0,                //    101
    0,                //    110
    0x1               //    111
};
static const UINT8 prs_fraction[8] =
{
    //                -AWGN_COEF_SET
    0x3,//0x0         //  8psk 3/5;
    0,                //    8psk 2/3;
    0,                //    010
    0x2,///0          //    011
    0,                //  for DVBS
    0,                //    101
    0,                //    110
    0                 //    111
};
static const UINT8 prs[8] =
{
    //                -AWGN_COEF_SET
    0x7,               //  8psk 3/5;
    0x9,               //    8psk 2/3;
    0x8,               //    010
    0x7,               //    011
    0x8,               //  for DVBS
    0x0,               //    101
    0x9,               //    110
    0x7                //    111
};
static const UINT8 irs_fraction[8] =
{
    //                -AWGN_COEF_SET
    0x3,    //0x0      //  8psk 3/5;
    0,                 //    8psk 2/3;
    0,                 //    010
    0x2,//0            //    011
    0,                 //  for DVBS
    0,                 //    101
    0,                 //    110
    0                  //    111
};
static const UINT8 irs[8] =
{
    //                -AWGN_COEF_SET
    0x13,              //  8psk 3/5;
    0x12,              //    8psk 2/3;
    0x11,              //    010
    0x10,              //    011
    0x11,              //  for DVBS
    0x0,               //    101
    0x12,              //    110
    0x13               //    111
};
//----------6.5M to 33M begin--------------------------
static const UINT8 symbol_before_head_extra[8] =
{
    //                        -AWGN_COEF_SET
    1,                // 8psk 3/5;
    1,                // 8psk 2/3;
    1,                //
    1,                //
    1,                // dont use
    1,                // dont use
    1,                // dotn use
    0                //
};
static const UINT8 head_ped_gain_extra[8] =
{
    //                        -AWGN_COEF_SET
    0x1,            //  8psk 3/5;
    0x1,            //    8psk 2/3;
    0x1,                //    010
    0x1,                //    011
    0,                //  100
    0,                //    101
    0,                //    110
    0x0                //    111
};
static const UINT8 prs_fraction_extra[8] =
{
    //                        -AWGN_COEF_SET
    0x3,//0x0                //  8psk 3/5;
    0,                //    8psk 2/3;
    0,                //    010
    0x2,///0                //    011
    0,                //  100
    0,                //    101
    0,                //    110
    0                //    111
};
static const UINT8 prs_extra[8] =
{
    //                        -AWGN_COEF_SET
    0x7,            //  8psk 3/5;
    0x9,            //    8psk 2/3;
    0x8,                //    010
    0x7,                //    011
    0xa,                //  100
    0xb,                //    101
    0x9,                //    110
    0x7                //    111
};
static const UINT8 irs_fraction_extra[8] =
{
    //                        -AWGN_COEF_SET
    0x3,    //0x0            //  8psk 3/5;
    0,                //    8psk 2/3;
    0,                //    010
    0x2,//0                //    011
    0,                //  100
    0,                //    101
    0,                //    110
    0                //    111
};
static const UINT8 irs_extra[8] =
{
    //                        -AWGN_COEF_SET
    0x13,            //  8psk 3/5;
    0x12,            //    8psk 2/3;
    0x11,                //    010
    0x10,                //    011
    0x13,                //  100
    0x14,                //    101
    0X12,                //    110
    0x13                //    111
};


static const UINT8 wider_prs_delta[8] =
{
    //                        -LOOP_COEF_SET
    0x2,                // dont touch
    0x2,                //dont touch
    0,                //    dont touch
    0x2,                //    011
    2,                //  dont touch
    0,                //    101
    0,                //    110
    0                //    dont touch
};
static const UINT8 wider_irs_delta[8] =
{
    //                        -LOOP_COEF_SET
    0x2,                //  dont touch
    0x2,                //    dont touch
    0,                //    dont touch
    0x2,                //    011
    2,                //  dont touch
    0,                //    101
    0,                //    110
    0                //    dont touch
};
static const UINT8 wider_snr_delta[8] =
{
    //                        -LOOP_COEF_SET
    0x7,                //  dont touch
    0x0,                //dont touch
    0x7,                //    dont touch
    0x5,                //    011
    0x7,                //  dont touch
    0x7,                //    101
    0x7,                //    110
    0x7                //    dont touch
};
static const UINT8 prs_step[8] =
{
    //                        -LOOP_COEF_SET
    0x3,            //  dont touch
    0x0,            //    dont touch
    0x4,            //    dont touch
    0x4,            //    011 1*7
    0x0,                //  dont touch
    0x6,                //    101 6*1
    0x2,                //    110 2*3
    0x7                //    dont touch
};
static const UINT8 irs_step[8] =
{
    //                        -LOOP_COEF_SET
    0x3 ,            //  dont touch
    0,            //    dont touch
    0x4,                //    dont touch
    0x4,                //    011
    0x0,                //  dont touch
    0x6,                //    101
    0x2,                //    110
    0x7                //    dont touch
};
static const UINT8 max_snr_delta[8] =
{
    //                        -LOOP_COEF_SET
    0x1,            //  dont touch
    0x1,            //    dont touch
    0x1,                //    dont touch
    0x2,                //    011 1*7
    0x1,                //  dont touch
    0x1,                //    101 6*1
    0x3,                //    110 2*3
    0x1                //    dont touch
};
static const UINT8 head_gain_snr_delta[8] =
{
    //                        -LOOP_COEF_SET
    0x4,            // dont touch
    0x0,            //    dont touch
    0x2,                //    dont touch
    0x2,                //    011
    0x2,                //  dont touch
    0x2,                //    101
    0x2,                //    110
    0x2                //    dont touch
};


static const UINT8 clip_ped2[8] =
{
    //                        -CLIP_PED_SET
    0x73,        //  8psk 3/5
    0x80,        //    8psk 2/3
    0x95,        //    010
    0x20,            //    011
    0x73,            //  100
    0,            //    101
    0,            //    110
    0            //    111

};
static const UINT8 clip_ped2_en[8] =
{
    //                        -CLIP_PED_SET
    0x1,        //  8psk 3/5
    0x1,        //    8psk 2/3
    0x0,            //    010
    0x1,            //    011
    0,            //  100
    0,            //    101
    0,            //    110
    0            //    111

};
static const UINT8 clip_ped1[8] =
{
    //                        -CLIP_PED_SET
    0x37,        //  8psk 3/5
    0x90,        //    8psk 2/3
    0x50,            //    8psk 3/4
    0x30,            //    011
    0x37,            //  100
    0,            //    101
    0,            //    110
    0            //    111

};
static const UINT8 clip_ped1_en[8] =
{
    //                        -CLIP_PED_SET
    0x1,        //  8psk 3/5
    0x1,        //    8psk 2/3
    0x0,            //    8psk 3/4
    0x1,            //    011
    0,            //  100
    0,            //    101
    0,            //    110
    0            //    111

};
static const UINT8 clip_mult_step[8] =
{
    //                        -CLIP_PED_SET
    0x0,        //  8psk 3/5
    0x0,        //    8psk 2/3
    0,            //    010
    0x0f,            //    011
    0,            //  100
    0,            //    101
    0,            //    110
    0            //    111

};

static const UINT8 avg_ped2[8] =
{
    //                        -AVG_PED_SET
    0x32,            //  8psk 3/5
    0x18,            //    8psk 2/3
    0x70,                //    010
    0x05,                //    011
    0x46,                //  100
    0x32,                //    101
    0,                //    110
    0                //    111
};
static const UINT8 m_avg_ped2_en[8] =
{
    //                        -AVG_PED_SET
    0x1,            //  8psk 3/5
    0x1,                //    8psk 2/3
    1,                //    010
    1,                //    011
    1,                //  100
    1,                //    101
    0,                //    110
    0                //    111
};
static const UINT8 avg_ped1[8] =
{
    //                        -AVG_PED_SET
    0x96,            //  8psk 3/5
    0x30,            //    8psk 2/3
    0xe0,                //    010
    0x0a,                //    011
    0x96,                //  100
    0x96,                //    101
    0,                //    110
    0                //    111
};
static const UINT8 avg_ped1_en[8] =
{
    //                        -AVG_PED_SET
    0x1,            //  8psk 3/5
    0x1,            //    8psk 2/3
    1,                //    010
    1,                //    011
    1,                //  100
    1,                //    101
    0,                //    110
    0                //    111
};
static const UINT8 avg_mult_step[8] =
{
    //                        -AVG_PED_SET
    0x0,            //  8psk 3/5
    0x0,            //    8psk 2/3
    0,                //    010
    0,                //    011
    0,                //  100
    0,                //    101
    0,                //    110
    0                //    111
};
static const UINT8 avg_snr_delta[8] =
{
    //                        -AVG_PED_SET
    0x1,            //  8psk 3/5
    0x1,            //    8psk 2/3
    0,                //
    0,                //
    0,                //
    0,                //
    0,                //
    0                //
};
static const UINT16 s_awgn_noise[6] = // dont need to change
{
    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
};

static const UINT8 S_AWGN_COEF_SET[6] =
{
    0x4,    0x4,    0x4,    0x4,    0x4,    0x4
};

static const UINT8 S_PON_IRS_DELTA[6] = //dont need to change
{
    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
};

static const UINT8 S_PON_PRS_DELTA[6] = //dont need to change
{
    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
};

static const UINT8 S_LOOP_COEF_SET[6] =
{
    0x4,0x4,0x4,0x4,0x4,0x4
};

static const UINT8 S_CLIP_PED_SET[6] =
{
    0x2,0x2,0x2,0x2,0x2,0x2
};

static const UINT8 S_AVG_PED_SET[6] =
{
    0x6,    0x6,    0x6,    0x6,    0x6,    0x6
};

static const UINT8 S_FORCE_OLD_CR[6] =
{
    0x1,    0x1,    0x1,    0x1,    0x1,    0x1
};

static const UINT8 S_LLR_SHIFT[6] =
{
    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
};
//-End: TAB for New ADPT Add by Hongyu //


/*****************************************************************************
* void nim_s3503_after_reset_set_param(struct nim_device *dev)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
void nim_s3503_after_reset_set_param(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT32 data_tmp = 0;
    int i = 0;
	struct nim_s3501_private *priv = NULL;
	
    if(dev == NULL)
    {
	    return;
	}	
    priv = (struct nim_s3501_private *) dev->priv;

    NIM_PRINTF(" Current version is %s\n",VERSION_S3503);
    nim_s3503_set_demod_ctrl(dev, NIM_DEMOD_CTRL_0X91);

    nim_s3503_interrupt_mask_clean(dev);
    data = 0x50;
    nim_reg_write(dev, R0A_AGC1_LCK_CMD, &data, 1);

    // set diseqc clock.
    nim_s3503_get_dsp_clk(dev, &data_tmp);
    // Set diseqc parameter
    data = ((data_tmp + 500) / 1000);
    nim_reg_write(dev, R90_DISEQC_CLK_RATIO, &data, 1);

    // set unconti timer
    data = 0x40;
    nim_reg_write(dev, RBA_AGC1_REPORT, &data, 1);

    //set receive timer: 0x88 for default;
    data = 0x88;
    nim_reg_write(dev, R8E_DISEQC_TIME, &data, 1);
    data = 0xff;
    nim_reg_write(dev, R8E_DISEQC_TIME + 0x01, &data, 1);
    // M3501C_DCN : may move those register to HW_INI
#ifdef DISEQC_OUT_INVERT
    // cr96[7] is DISEQC invert
    nim_reg_read(dev, R90_DISEQC_CLK_RATIO + 6, &data, 1);
    data = data | 0x80;
    nim_reg_write(dev, R90_DISEQC_CLK_RATIO + 6, &data, 1);
#endif
    // Open new tso
    // bit0: tso_bist, bit1, insert dummy, bit2, sync_lenght, bit3, vld gate,
    // bit 4, cap_eco_en, bit5, cap pid enable ,
    // bit[7:6] dsp clk sel: 00: 90m, 01:98m, 10:108m, 11:135m
    data = 0x12;
    nim_reg_write(dev, RF0_HW_TSO_CTRL, &data, 1);

    // set TSO fifo burst mode (input 8 byte start output)
    nim_reg_read(dev, RF1_DSP_CLK_CTRL, &data, 1);
    data = data | 0x40;
    nim_reg_write(dev, RF1_DSP_CLK_CTRL, &data, 1);

    // TSO OEJ
    nim_reg_read(dev, RF0_HW_TSO_CTRL + 5, &data, 1);
    data = data & 0xef;
    nim_reg_write(dev, RF0_HW_TSO_CTRL + 5, &data, 1);

    // Set TSO bitrate margin, unit 1/16M,
    data = 0x08;
    nim_reg_write(dev, RF0_HW_TSO_CTRL + 2, &data, 1);
    for (i = 0; i < 32; i++)
    {
        data = i;
        nim_reg_write(dev, R9C_DEMAP_BETA + 0x02, &data, 1);
        data = map_beta_active_buf[i];
        nim_reg_write(dev, R9C_DEMAP_BETA, &data, 1);
        data = 0x04;
        nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
        // For C3503 only
        if(priv->ul_status.m_s3501_sub_type == NIM_C3503_SUB_ID)
        {
            if(s2_llr_shift[i])
            {
            data = map_beta_buf_llr_shift_on[i];
            }
            else
            {
            data = map_beta_buf_llr_shift_off[i];
            }
        }
        else
        {
        data = map_beta_buf_llr_shift_off[i];
        }

        nim_reg_write(dev, R9C_DEMAP_BETA, &data, 1);
        data = 0x03;
        nim_reg_write(dev, R9C_DEMAP_BETA + 0x03, &data, 1);
    }


    // FEC use SPI output to TSO, use fast speed
    data = 0x10;
    nim_reg_write(dev, RAD_TSOUT_SYMB + 1, &data, 1);
    data = 0x02;
    nim_reg_write(dev, RAD_TSOUT_SYMB, &data, 1);
    return ;
}


/*****************************************************************************
*static void nim_s3503_fec_set_demap_noise(struct nim_device *dev)
* Description: S3501 set polarization
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8 pol
*
* Return Value: void
*****************************************************************************/
void nim_s3503_fec_set_demap_noise(struct nim_device *dev)
{
    UINT8 data = 0;
    UINT8 noise_index = 0;
    UINT16 est_noise = 0;

    if(dev == NULL)
    {
	    return;
	}	
    // activate noise
    nim_reg_read(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);
    data &= 0xfc;
    nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 2, &data, 1);

    // set noise_index
    noise_index = 0x0c; // 8psk,3/5.
    nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &noise_index, 1);

    // set noise
    est_noise = demap_noise[noise_index];
    data = est_noise & 0xff;
    nim_reg_write(dev, RD0_DEMAP_NOISE_RPT, &data, 1);
    data = (est_noise >> 8) & 0x3f;
    data |= 0xc0;
    nim_reg_write(dev, RD0_DEMAP_NOISE_RPT + 1, &data, 1);
    //  }
}



INT32 nim_s3503_cr_new_tab_init(struct nim_device *dev)
{
    UINT8 tabid = 0;
    UINT8 cellid = 0;
    UINT8 data = 0;

    UINT32 s2_code_rate_table[28]={0};
    UINT32 awgn_coef_table[8]={0};
    UINT32 loop_coef_table[8]={0};
    UINT32 clip_ped_table[8]={0};
    UINT32 avg_ped_table[8]={0};
    UINT32 s_code_rate_table[6]={0};

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	
    ADPT_NEW_CR_PRINTF("NEW CR TAB Initialization Begin \n");

    for(cellid = 0; cellid < 28; cellid++)
    {
        //        cellid=11;
        tabid =  0;
        s2_code_rate_table[cellid] = ((((s2_awgn_noise[cellid]) << 1) & 0x00000ffe) << 20) | // why?
                                     ((s2_awgn_coef_set[cellid] & 0x00000007) << 17) |
                                     ((s2_pon_irs_delta[cellid] & 0x00000007) << 14) |
                                     ((s2_pon_prs_delta[cellid] & 0x00000007) << 11) |
                                     ((s2_loop_coef_set[cellid] & 0x00000007) << 8) |
                                     ((s2_cup_ped_set[cellid] & 0x00000007) << 5) |
                                     ((s2_avg_ped_set[cellid] & 0x00000007) << 2) |
                                     ((s2_force_old_cr[cellid] & 0x00000001) << 1) |
                                     (s2_llr_shift[cellid] & 0x00000001);

		nim_s3503_set_cr_new_value(dev,tabid,cellid,s2_code_rate_table[cellid]);  
    }

    for(cellid = 0; cellid < 8; cellid++)
    {
        tabid =  1;
        //        cellid=0;
        awgn_coef_table[cellid] = ((irs[cellid] & 0x0000001f) << 12)  |
                                  ((irs_fraction[cellid] & 0x00000003) << 10) |
                                  ((prs[cellid] & 0x0000000f) << 6) |
                                  ((prs_fraction[cellid] & 0x00000003) << 4) |
                                  ((head_ped_gain[cellid] & 0x00000007) << 1) |
                                  (symbol_before_head[cellid] & 0x00000001);

        nim_s3503_set_cr_new_value(dev,tabid,cellid,awgn_coef_table[cellid]);  
    }

    for(cellid = 0; cellid < 8; cellid++)
    {
        //        cellid=0;
        tabid =  2;
        loop_coef_table[cellid] = ((head_gain_snr_delta[cellid] & 0x00000007) << 16) |
                                  ((max_snr_delta[cellid] & 0x00000007) << 13) |
                                  ((irs_step[cellid] & 0x00000007) << 10) |
                                  ((prs_step[cellid] & 0x00000007) << 7) |
                                  ((wider_snr_delta[cellid] & 0x00000007) << 4) |
                                  ((wider_irs_delta[cellid] & 0x00000003) << 2) |
                                  (wider_prs_delta[cellid] & 0x00000003);

        nim_s3503_set_cr_new_value(dev,tabid,cellid,loop_coef_table[cellid]);  
    }


    for(cellid = 0; cellid < 8; cellid++)
    {
        tabid =  3;
        clip_ped_table[cellid] = ((clip_mult_step[cellid] & 0x0000001f) << 18) |
                                 ((clip_ped1_en[cellid]  & 0x00000001) << 17) |
                                 ((clip_ped1[cellid] & 0x000000ff) << 9) |
                                 ((clip_ped2_en[cellid] & 0x00000001) << 8)    |
                                 (clip_ped2[cellid] & 0x000000ff);

        nim_s3503_set_cr_new_value(dev,tabid,cellid,clip_ped_table[cellid]);  
    }


    for(cellid = 0; cellid < 8; cellid++)
    {
        tabid =  4;
        avg_ped_table[cellid] = ((avg_snr_delta[cellid] & 0x00000007) << 23) |
                                ((avg_mult_step[cellid] & 0x0000001f) << 18) |
                                ((avg_ped1_en[cellid] & 0x00000001) << 17) |
                                ((avg_ped1[cellid] & 0x000000ff) << 9) |
                                ((m_avg_ped2_en[cellid] & 0x00000001) << 8)    |
                                (avg_ped2[cellid] & 0x000000ff);

       nim_s3503_set_cr_new_value(dev,tabid,cellid,avg_ped_table[cellid]);  
    }

    for(cellid = 0; cellid < 6; cellid++)
    {
        tabid =  5;
        s_code_rate_table[cellid] = ((((s_awgn_noise[cellid]) << 1) & 0x00000ffe) << 20) | // why?
                                    ((S_AWGN_COEF_SET[cellid] & 0x00000007) << 17) |
                                    ((S_PON_IRS_DELTA[cellid] & 0x00000007) << 14) |
                                    ((S_PON_PRS_DELTA[cellid] & 0x00000007) << 11) |
                                    ((S_LOOP_COEF_SET[cellid] & 0x00000007) << 8) |
                                    ((S_CLIP_PED_SET[cellid] & 0x00000007) << 5) |
                                    ((S_AVG_PED_SET[cellid] & 0x00000007) << 2) |
                                    ((S_FORCE_OLD_CR[cellid] & 0x00000001) << 1) |
                                    (S_LLR_SHIFT[cellid] & 0x00000001);

       nim_s3503_set_cr_new_value(dev,tabid,cellid,s_code_rate_table[cellid]);  
    }

    // setting SNR_EST_LEN to 7, so that the value of EST_NOISE can keep stable.
    nim_reg_read(dev, R113_NEW_CR_ADPT_CTRL, &data, 1);
    data |= 0x70;
    nim_reg_write(dev, R113_NEW_CR_ADPT_CTRL, &data, 1);

    ADPT_NEW_CR_PRINTF("NEW CR TAB Initialization Done \n");

    return SUCCESS;
}


INT32 nim_s3503_cr_new_modcod_table_init(struct nim_device *dev, UINT32 sym)
{

    UINT8 tabid = 0;
    UINT8 cellid = 0;
    UINT16 tabval_14_0 = 0;
    UINT16 tabval_29_15 = 0;
    UINT8 data = 0;
    UINT8 datarray[2]={0};
    UINT32 awgn_coef_table[8]={0};

    UINT16 s2_awgn_noise_tmp = 0;
    UINT8 s2_awgn_coef_set_tmp = 0;
    UINT8 s2_pon_irs_delta_tmp = 0;
    UINT8 s2_pon_prs_delta_tmp = 0;
    UINT8 s2_loop_coef_set_tmp = 0;
    UINT8 s2_clip_ped_set_tmp = 0;
    UINT8 s2_avg_ped_set_tmp = 0;
    UINT8 s2_force_old_cr_tmp = 0;

    UINT8 irs_tmp = 0;
    UINT8 irs_fraction_tmp = 0;
    UINT8 prs_tmp = 0;
    UINT8 prs_fraction_tmp = 0;
    UINT8 head_ped_gain_tmp = 0;
    UINT8 symbol_before_head_tmp = 0;

    UINT32 s2_code_rate_table[28]={0};

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    for(cellid = 0; cellid < 28; cellid++)
    {
        tabid =  0;
        if((SYM_6500 < sym) && (sym < SYM_33000))
        {
            s2_awgn_noise_tmp = S2_AWGN_NOISE_EXTRA[cellid];
            s2_pon_irs_delta_tmp = s2_pon_irs_delta_extra[cellid];
            s2_pon_prs_delta_tmp = s2_pon_prs_delta_extra[cellid];
            s2_clip_ped_set_tmp = s2_cup_ped_set_extra[cellid];
            s2_avg_ped_set_tmp = s2_avg_ped_set_extra[cellid];
            s2_awgn_coef_set_tmp = s2_awgn_coef_set_extra[cellid];
            s2_loop_coef_set_tmp = s2_loop_coef_set_extra[cellid];
            s2_force_old_cr_tmp = s2_force_old_cr[cellid];
            if((cellid == CELL_ID_13) && (sym < SYM_16000)) // patch for 8psk R3_4 using 5M parameters
            {
                s2_loop_coef_set_tmp = 0x0; // LOOP_COEF_table dont EXTRA
                s2_awgn_coef_set_tmp = 0x7;
                s2_force_old_cr_tmp = 0x1;
            }
            if((cellid == CELL_ID_11) && (sym < SYM_16000)) //pathc for 8psk R3_5
            {
                s2_awgn_coef_set_tmp = 0x6; //
                s2_avg_ped_set_tmp = 0x6;
                s2_force_old_cr_tmp = 0x1;
                s2_pon_prs_delta_tmp = 0;
                s2_pon_irs_delta_tmp = 0;
            }
            if((cellid == CELL_ID_12) && (sym < SYM_16000)) // patch for 8psk R2_3
            {
                s2_awgn_coef_set_tmp = 0x6;
                //s2_awgn_noise_tmp=0x8c;//0xa0
                s2_loop_coef_set_tmp = 0x5;
                s2_force_old_cr_tmp = 0x1;
            }
            if((cellid == CELL_ID_14) && (sym < SYM_16000)) // for 8psk R5_6
            {
                s2_awgn_coef_set_tmp = 0x7;
                s2_loop_coef_set_tmp = 0x6;
            }
            if((cellid == CELL_ID_3) && (sym < SYM_12000)) // for qpsk R1_2
            {
                // s2_force_old_cr_tmp=0x1;
                s2_awgn_coef_set_tmp = 0x1;
                s2_pon_prs_delta_tmp = 0x7; // in order to not infulence pilot on
                s2_pon_irs_delta_tmp = 0x7;
                s2_awgn_noise_tmp = 0x1c9;// only for 6.5M to 16M
            }
        }
        else
        {
            s2_awgn_noise_tmp = s2_awgn_noise[cellid];
            s2_awgn_coef_set_tmp = s2_awgn_coef_set[cellid];
            s2_loop_coef_set_tmp = s2_loop_coef_set[cellid];
            s2_clip_ped_set_tmp = s2_cup_ped_set[cellid];
            s2_avg_ped_set_tmp = s2_avg_ped_set[cellid];
            s2_pon_irs_delta_tmp = s2_pon_irs_delta[cellid];
            s2_pon_prs_delta_tmp = s2_pon_prs_delta[cellid];
            s2_force_old_cr_tmp = s2_force_old_cr[cellid];
            if((cellid == CELL_ID_11) && (sym < SYM_4000)) // patch for 8psk R3_5 symbol_rate<4000
            {
                s2_pon_irs_delta_tmp = 0x6;
                s2_pon_prs_delta_tmp = 0x6;
                ADPT_NEW_CR_PRINTF("below 4000\n");
            }
            if((cellid == CELL_ID_3) && (sym <= SYM_6500)) // for qpsk R1_2
            {
                s2_awgn_noise_tmp = 0x1ed;
                s2_awgn_coef_set_tmp = 0x1;
            }
        }
        if(cellid == CELL_ID_11)
        {
            ADPT_NEW_CR_PRINTF("s2_pon_irs_delta_tmp is %d\n", s2_pon_irs_delta_tmp);
        }
        s2_code_rate_table[cellid] = ((((s2_awgn_noise_tmp) << 1) & 0x00000ffe) << 20) | // why?
                                     ((s2_awgn_coef_set_tmp & 0x00000007) << 17) |
                                     ((s2_pon_irs_delta_tmp & 0x00000007) << 14) |
                                     ((s2_pon_prs_delta_tmp & 0x00000007) << 11) |
                                     ((s2_loop_coef_set_tmp & 0x00000007) << 8) |
                                     ((s2_clip_ped_set_tmp & 0x00000007) << 5) |
                                     ((s2_avg_ped_set_tmp & 0x00000007) << 2) |
                                     ((s2_force_old_cr_tmp & 0x00000001) << 1) |
                                     (s2_llr_shift[cellid] & 0x00000001);
        //step 1:
        data = (tabid << 5) | (cellid & 0x1f);
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1 );
        //step 2:
        tabval_14_0 = (UINT16)((s2_code_rate_table[cellid]  & 0x7fff));
        datarray[0] = (UINT8)tabval_14_0;
        datarray[1] = ((UINT8)(tabval_14_0 >> 8)) & 0x7f;
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

        //step 3:
        tabval_29_15 = (UINT16)(s2_code_rate_table[cellid] >> 15);
        datarray[0] = (UINT8)(tabval_29_15);
        datarray[1] = (UINT8)(tabval_29_15 >> 8) | 0x80; // enable CR_PARA_WE_2
        nim_reg_write(dev, R130_CR_PARA_DIN, datarray, 2);
    }

    for(cellid = 0; cellid < 8; cellid++)
    {
        tabid =  1;
        if((SYM_6500 < sym) && (sym < SYM_33000))
        {

            irs_tmp = irs_extra[cellid];
            irs_fraction_tmp = irs_fraction_extra[cellid];
            prs_tmp = prs_extra[cellid];
            prs_fraction_tmp = prs_fraction_extra[cellid];
            head_ped_gain_tmp = head_ped_gain_extra[cellid];
            symbol_before_head_tmp = symbol_before_head_extra[cellid];
            ADPT_NEW_CR_PRINTF("6500<sym<33000\n");
        }
        else
        {
            irs_tmp = irs[cellid];
            irs_fraction_tmp = irs_fraction[cellid];
            prs_tmp = prs[cellid];
            prs_fraction_tmp = prs_fraction[cellid];
            head_ped_gain_tmp = head_ped_gain[cellid];
            symbol_before_head_tmp = symbol_before_head[cellid];
            ADPT_NEW_CR_PRINTF("sym<6500 or sym>33000\n");
        }
        awgn_coef_table[cellid] = ((irs_tmp & 0x0000001f) << 12)  |
                                  ((irs_fraction_tmp & 0x00000003) << 10) |
                                  ((prs_tmp & 0x0000000f) << 6) |
                                  ((prs_fraction_tmp & 0x00000003) << 4) |
                                  ((head_ped_gain_tmp & 0x00000007) << 1) |
                                  (symbol_before_head_tmp & 0x00000001);
        //step 1:
        data = (tabid << 5) | (cellid & 0x1f);
        nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1 );

        //step 2:
        tabval_14_0 = (UINT16)((awgn_coef_table[cellid]  & 0x7fff));
        datarray[0] = (UINT8)tabval_14_0;
        datarray[1] = ((UINT8)(tabval_14_0 >> 8)) & 0x7f;
        nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

        //step 3:
        tabval_29_15 = (UINT16)(awgn_coef_table[cellid] >> 15);
        datarray[0] = (UINT8)(tabval_29_15);
        datarray[1] = (UINT8)(tabval_29_15 >> 8) | 0x80; // enable CR_PARA_WE_2
        nim_reg_write(dev, R130_CR_PARA_DIN, datarray, 2);

    }

    ADPT_NEW_CR_PRINTF("Re-inital S2_CODE_RATE_table Done\n");
    return SUCCESS;
}



/******************************************************************************
   *
static INT32 nim_s3503_fec_llr_shift(struct nim_device *dev)
*   function describle:
*    if this fuction is execute, it will detect reg_cr113[0]. if reg_cr113[0] equal to one, it will re-initial
*    NEW CR parameter TAB about LLR_SHIFT to one.
*    if reg_cr113[0] equal to zero, it will re-initial NEW CR parameter TAB about LLR_SHIFT to zero
*
******************************************************************************/
INT32 nim_s3503_fec_llr_shift(struct nim_device *dev)
{
    UINT32 s2_code_rate_table[28]={0};
    UINT32 s_code_rate_table[6]={0};
    UINT8 cellid = 0;
    UINT8 tabid = 0;
    UINT8 data = 0;
    UINT8 datarray[2]={0};
    UINT16 tabval_14_0 = 0;
    UINT16 tabval_29_15 = 0;
    UINT8 reg_llr_shift = 0;
    UINT8 rdata = 0;

    if(dev == NULL)
    {
	    return RET_FAILURE;
	}	

    nim_reg_read(dev, 0x113, &rdata, 1);
    rdata &= 0x01;
    if(last_value != rdata)
    {
        last_value = rdata;
        if(rdata == 0x01) // enable LLR_SHIFT
        {
            /*
            1) S2_CODE_RATE_TABLE size = 28*30 = 840 bits
            index = {code_rate}, 28 index
            value = { //30 bits
                S2_AWGN_NOISE,             //10 bits, EST_NOISE (MSBs) level to apply AWGN coefficients
                S2_AWGN_COEF_SET,  //3 bit, select one set from the AWGN_COEF table
                S2_PON_IRS_DELTA,  //3 bit signed, subtract from AWGN IRS when Pilot On
                S2_PON_PRS_DELTA,  //3 bit signed, subtract from AWGN PRS when Pilot On
                S2_LOOP_COEF_SET,  //3 bit, select one set from the CLIP_PED table
                S2_CLIP_PED_SET,   //3 bit, select one set from the CLIP_PED table
                S2_AVG_PED_SET,    //3 bit, select one set from the AVG_PED table
                S2_FORCE_OLD_CR,   //1 bit, only affect pilot off
                S2_LLR_SHIFT ,             //1 bit
                }
            */
            // enable MERGE_ROUND
            nim_reg_read(dev, 0x137, &rdata, 1);
            rdata |= 0x20;
            nim_reg_write(dev, 0x137, &rdata, 1);
            nim_reg_read(dev, 0x113, &reg_llr_shift, 1);
            reg_llr_shift |= 0x80;
            nim_reg_write(dev, 0x113, &reg_llr_shift, 1);
            for(cellid = 0; cellid < 28; cellid++)
            {
                //        cellid=11;
                tabid =  0;
                s2_code_rate_table[cellid] = ((((s2_awgn_noise[cellid]) << 1) & 0x00000ffe) << 20) | // why?
                                             ((s2_awgn_coef_set[cellid] & 0x00000007) << 17) |
                                             ((s2_pon_irs_delta[cellid] & 0x00000007) << 14) |
                                             ((s2_pon_prs_delta[cellid] & 0x00000007) << 11) |
                                             ((s2_loop_coef_set[cellid] & 0x00000007) << 8) |
                                             ((s2_cup_ped_set[cellid] & 0x00000007) << 5) |
                                             ((s2_avg_ped_set[cellid] & 0x00000007) << 2) |
                                             ((s2_force_old_cr[cellid] & 0x00000001) << 1) |
                                             0x00000001;
                //step 1:
                data = (tabid << 5) | (cellid & 0x1f);
                nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1 );

                //step 2:
                tabval_14_0 = (UINT16)((s2_code_rate_table[cellid]  & 0x7fff));    
                datarray[0] = (UINT8)tabval_14_0;
                datarray[1] = ((UINT8)(tabval_14_0 >> 8)) & 0x7f;
                nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

                //step 3:
                tabval_29_15 = (UINT16)(s2_code_rate_table[cellid] >> 15);
                datarray[0] = (UINT8)(tabval_29_15);
                datarray[1] = (UINT8)(tabval_29_15 >> 8) | 0x80; // enable CR_PARA_WE_2
                nim_reg_write(dev, R130_CR_PARA_DIN, datarray, 2);
            }

            for(cellid = 0; cellid < 5; cellid++)
            {
                //        cellid=11;
                tabid =  5;
                s_code_rate_table[cellid] = ((((s_awgn_noise[cellid]) << 1) & 0x00000ffe) << 20) | // why?
                                            ((S_AWGN_COEF_SET[cellid] & 0x00000007) << 17) |
                                            ((S_PON_IRS_DELTA[cellid] & 0x00000007) << 14) |
                                            ((S_PON_PRS_DELTA[cellid] & 0x00000007) << 11) |
                                            ((S_LOOP_COEF_SET[cellid] & 0x00000007) << 8) |
                                            ((S_CLIP_PED_SET[cellid] & 0x00000007) << 5) |
                                            ((S_AVG_PED_SET[cellid] & 0x00000007) << 2) |
                                            ((S_FORCE_OLD_CR[cellid] & 0x00000001) << 1) |
                                            0x00000001;
                //step 1:
                data = (tabid << 5) | (cellid & 0x1f);
                nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1 );

                //step 2:
                tabval_14_0 = (UINT16)((s_code_rate_table[cellid]  & 0x7fff));
                datarray[0] = (UINT8)tabval_14_0;
                datarray[1] = ((UINT8)(tabval_14_0 >> 8)) & 0x7f;
                nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

                //step 3:
                tabval_29_15 = (UINT16)(s_code_rate_table[cellid] >> 15);
                datarray[0] = (UINT8)(tabval_29_15);
                datarray[1] = (UINT8)(tabval_29_15 >> 8) | 0x80; // enable CR_PARA_WE_2
                nim_reg_write(dev, R130_CR_PARA_DIN, datarray, 2);
            }
        }
        else
        {
            // disable MERGE_ROUND
            nim_reg_read(dev, 0x137, &rdata, 1);
            rdata &= 0xcf;
            nim_reg_write(dev, 0x137, &reg_llr_shift, 1);
            nim_reg_read(dev, 0x113, &reg_llr_shift, 1);
            reg_llr_shift &= 0x7f;
            nim_reg_write(dev, 0x113, &reg_llr_shift, 1);
            for(cellid = 0; cellid < 28; cellid++)
            {
                //        cellid=11;s2_pon_prs_delta
                tabid =  0;
                s2_code_rate_table[cellid] = ((((s2_awgn_noise[cellid]) << 1) & 0x00000ffe) << 20) | // why?
                                             ((s2_awgn_coef_set[cellid] & 0x00000007) << 17) |
                                             ((s2_pon_irs_delta[cellid] & 0x00000007) << 14) |
                                             ((s2_pon_prs_delta[cellid] & 0x00000007) << 11) |
                                             ((s2_loop_coef_set[cellid] & 0x00000007) << 8) |
                                             ((s2_cup_ped_set[cellid] & 0x00000007) << 5) |
                                             ((s2_avg_ped_set[cellid] & 0x00000007) << 2) |
                                             ((s2_force_old_cr[cellid] & 0x00000001) << 1) |
                                             0x00000000;
                //step 1:
                data = (tabid << 5) | (cellid & 0x1f);
                nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1 );

                //step 2:
                tabval_14_0 = (UINT16)((s2_code_rate_table[cellid]  & 0x7fff));
                datarray[0] = (UINT8)tabval_14_0;
                datarray[1] = ((UINT8)(tabval_14_0 >> 8)) & 0x7f;
                nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

                //step 3:
                tabval_29_15 = (UINT16)(s2_code_rate_table[cellid] >> 15);
                datarray[0] = (UINT8)(tabval_29_15);
                datarray[1] = (UINT8)(tabval_29_15 >> 8) | 0x80; // enable CR_PARA_WE_2
                nim_reg_write(dev, R130_CR_PARA_DIN, datarray, 2);
            }
            for(cellid = 0; cellid < 5; cellid++)
            {
                //        cellid=11;
                tabid =  5;
                s_code_rate_table[cellid] = ((((s_awgn_noise[cellid]) << 1) & 0x00000ffe) << 20) | // why?
                                            ((S_AWGN_COEF_SET[cellid] & 0x00000007) << 17) |
                                            ((S_PON_IRS_DELTA[cellid] & 0x00000007) << 14) |
                                            ((S_PON_PRS_DELTA[cellid] & 0x00000007) << 11) |
                                            ((S_LOOP_COEF_SET[cellid] & 0x00000007) << 8) |
                                            ((S_CLIP_PED_SET[cellid] & 0x00000007) << 5) |
                                            ((S_AVG_PED_SET[cellid] & 0x00000007) << 2) |
                                            ((S_FORCE_OLD_CR[cellid] & 0x00000001) << 1) |
                                            0x00000000;
                //step 1:
                data = (tabid << 5) | (cellid & 0x1f);
                nim_reg_write(dev, R9D_RPT_DEMAP_BETA, &data, 1 );

                //step 2:
                tabval_14_0 = (UINT16)((s_code_rate_table[cellid]  & 0x7fff));
                datarray[0] = (UINT8)tabval_14_0;
                datarray[1] = ((UINT8)(tabval_14_0 >> 8)) & 0x7f;
                nim_reg_write(dev, R11_DCC_OF_I, datarray, 2);

                //step 3:
                tabval_29_15 = (UINT16)(s_code_rate_table[cellid] >> 15);
                datarray[0] = (UINT8)(tabval_29_15);
                datarray[1] = (UINT8)(tabval_29_15 >> 8) | 0x80; // enable CR_PARA_WE_2
                nim_reg_write(dev, R130_CR_PARA_DIN, datarray, 2);
            }
        }

    }
    return SUCCESS;
}



