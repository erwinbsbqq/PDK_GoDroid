#include <basic_types.h>
#include "../include/hdmi_api.h"
#include "../include/ticks.h"
#include "../include/osal.h"

#define HDMI_BASE_ADDR                                          0xb802a000
#define hdmi_WriteByte(offset,value)            *(volatile UINT8 *)(HDMI_BASE_ADDR + offset) = (value)
#define hdmi_ReadByte(offset)                           (*((volatile UINT8*)(HDMI_BASE_ADDR+offset)))

/* hdmi register address definition */
enum HDMI_REG_ADDR
{
        R_VEN_ID                        = 0x00,
        R_DEV_ID                        = 0x02,
        R_DEV_REV                       = 0x04,
        R_INT                           = 0x05,
        R_INT_MASK                      = 0x06,
        R_CTRL                          = 0x07,
        R_STATUS                        = 0x08,
        R_CFG0                          = 0x09,
        R_CFG1                          = 0x0A,
        R_CFG2                          = 0x0B,
        R_CFG3                          = 0x0C,
        R_CFG4                          = 0x0D,
        R_CFG5                          = 0x0E,
        R_CFG6                          = 0x0F, // Add @S3602F
        R_WR_BKSV                       = 0x10,
        R_WR_AN                         = 0x15,
        R_RD_AKSV                       = 0x1D,
        R_RI                            = 0x22,
        R_KEYPORT                       = 0x24,
        R_KSVLIST                       = 0x25,
        R_BIST                          = 0x27,
        R_HDCP_STA                      = 0x2E,
        R_HDCP_CTL                      = 0x2F,
        R_V_VECTER                      = 0x30,
        R_I2S_C_STA                     = 0x50,
        R_I2S_UV                        = 0x55,
        R_CTS_CTRL                      = 0x58,
        R_CTS_VALUE                     = 0x59,
        R_NCTS                          = 0x61,
        R_IFM_PORT                      = 0x62,
        R_IFM_VER                       = 0x67,
        R_IFM_TYPE                      = 0x68,
        R_IFM_LENG                      = 0x69,
        R_DRV_CTL                       = 0x6A,
        R_PJ_REG                        = 0x6C,
        R_OPT                           = 0x6D,
        R_OPT1                          = 0x6E,
        R_OPT2                          = 0x6F,
        R_SRAM_ADDL                     = 0x70,
        R_CEC_STA                       = 0x71,
        R_SRAM_DATA                     = 0x72,
        R_CEC_BLK1                      = 0x74,
        R_CEC_BLK2                      = 0x75,
        R_DAT_BLK                       = 0x76,
        R_CEC_DATA1                     = 0x77,
        R_CEC_DATA2                     = 0x78,
        R_BIU_FREQ                      = 0x79,
        R_DIV_NUM                       = 0x7A,
        R_DUMY_ADDR                     = 0xFF,
};

/**********************************************/
/*                      Bit Field Definition                  */
/**********************************************/

/* offset 0x05, 0x06 */
#define B_INT_MDI                                       0x01
#define B_INT_HDCP                                      0x02
#define B_INT_FIFO_O                            0x04
#define B_INT_FIFO_U                            0x08
#define B_INT_IFM_ERR                           0x10
#define B_INT_INF_DONE                          0x20
#define B_INT_NCTS_DONE                 0x40
#define B_INT_CTRL_PKT_DONE             0x80

/* offset 0x07 */
#define B_SRST                                          0x01
#define B_PDB                                           0x02
#define B_GENERIC_EN                            0x04
#define B_SPD_EN                                        0x08
#define B_MPEG_EN                                       0x10
#define B_AUDIO_EN                                      0x20
#define B_AVI_EN                                        0x40
#define B_AV_MUTE                                       0x80

/* offset 0x08 */
#define B_HTPLG                                         0x01
#define B_PORD                                          0x02
#define B_AUD_SAMPLE_DONE                       0x04
#define B_GENERIC_DONE                          0x08
#define B_SPD_DONE                                      0x10
#define B_MPEG_DONE                             0x20
#define B_AUDIO_DONE                            0x40
#define B_AVI_DONE                                      0x80

/* offset 0x09 */
#define B_I2S_MODE                                      0x03
#define B_LRCK                                          0x04
#define AV_UNMUTE                                       0x08
#define B_W_LENGTH                                      0x30
#define B_VSYNC_SEL                                     0x40

/* offset 0x0A */
#define B_SPDIF                                         0x02
#define B_DVI                                           0x04
#define B_ONE_BIT_AUD                           0x08
#define B_ANA_RST                                       0x10
#define B_RAMP_EN                                       0x20
#define B_LFSR_EN                                       0x40
#define B_CK_SEL                                                0x80

/* offset 0x0B */
#define B_TXP_CTL0_2_0                          0x07
#define B_TXN_CTL0_2_0                          0x38
#define B_TXP_CTL1_1_0                          0xC0

/* offset 0x0C */
#define B_TXP_CTL1_2                            0x01
#define B_TXN_CTL1_2_0                          0x0E
#define B_TXP_CTL2_2_0                          0x70
#define B_TXN_CTL2_0                            0x80

/* offset 0x0D */
#define B_TXN_CTL2_2_1                          0x03
#define B_TXP_CTL3_2_0                          0x1C
#define B_TXN_CTL3_2_0                          0xE0

/* offset 0x0E */
#define B_PLL_SEL0                                      0x01
#define B_PLL_SEL1                                      0x02
#define B_ANA_PD                                        0x04
#define B_PDCLK                                         0x08
#define B_AUD_FLAT_BIT                          0x20
#define B_MUTETYPE_SEL                          0x40
#define B_T_SEL                                         0x80

#define B_AUDIO_CAP_RST                 0x01
#define B_EMP_EN                                        0x0E
#define B_PLL_SEL2                                      0x10
/* offset 0x2E */
#define B_ENC_ON                                        0x01
#define B_BKSV_ERR                                      0x02
#define B_RI_RDY                                                0x04
#define B_V_MATCH                                       0x08
#define B_V_RDY                                         0x10
#define B_AKSV_RDY                                      0x20
#define B_PJ_RDY                                                0x40

/* offset 0x2F */
#define B_ENC_EN                                        0x01
#define B_AUTHEN_EN                             0x02
#define B_CP_RST                                        0x04
#define B_AN_STOP                                       0x08
#define B_RX_RPTR                                       0x10
#define B_SCRAMBLE                                      0x20
#define B_HOST_KEY                                      0x40
#define B_SHA_EN                                        0x80

/* offset 0x55 */
#define B_USER_BIT                                      0x01
#define B_VALIDITY_BIT                          0x02
#define B_CH_EN                                         0x3C
#define B_NORMAL_MONO_SEL                       0x40
#define B_RGB_YCBCR_MONO                        0x80    // hdmi mono output color space select

/* offset 0x58 */
#define B_SOFT                                          0x01

/* offset 0x6D */
#define B_AC                                                    0x01
#define B_ELINK                                         0x02
#define B_ACP_EN                                        0x04
#define B_ACP_DONE                                      0x08
#define B_ISRC1_EN                                      0x10
#define B_ISRC1_DONE                            0x20
#define B_ISRC2_EN                                      0x40
#define B_ISRC2_DONE                            0x80

/* offset 0x6E */
#define B_CEC_ADDR                                      0x0F
#define B_CEC_MSK                                       0x20
#define B_PRO_SEL                                       0x40
#define B_PRO_SEL1                                      0x80

/* offset 0x6F */
#define B_HI2LO                                         0x01
#define B_LD_KSV                                        0x02
#define B_CEC_RST                                       0x04
#define B_CTL3_EN                                       0x08
#define B_RAM_EN                                        0x10
#define B_RAM_WR                                        0x20
#define B_WD_ADDRH                                      0x80

/* offset 0x71 */
#define B_RCV_UNKNOW                            0x01
#define B_STR_RISEDG                            0x02
#define B_STR_FALEDG                            0x04
#define B_BLK_RISEDG                            0x08
#define B_BLK_FALEDG                            0x10
#define B_Q1_CLR                                        0x20
#define B_Q2_CLR                                        0x40
#define B_REFILL                                                0x80

/* offset 0x74 */
#define B_CEC_BLK1_8_2                          0x7F
#define B_BLK1_EN                                       0x80

/* offset 0x75 */
#define B_CEC_BLK2_8_2                          0x7F
#define B_BLK2_EN                                       0x80

/* offset 0x76 */
#define B_CEC_BLK1_1_0                          0x03
#define B_CEC_BLK2_1_0                          0x0C
#define B_CEC_DAT1_1_0                          0x30
#define B_CEC_DAT2_1_0                          0xC0

/* offset 0x77 */
#define B_CEC_DAT1_8_2                          0x7F
#define B_DAT1_RDY                                      0x80

/* offset 0x78 */
#define B_CEC_DAT2_8_2                          0x7F
#define B_DAT2_RDY                                      0x80

/* offset 0x7A */
#define B_DIV_NUM                                       0x3F
#define B_BYPASS                                        0x40
#define B_BRDCST                                        0x80

/*******************************************************************
*               Each parameter definition for AVI InfoFrame
*******************************************************************/
enum AVI_IFM_FORMAT
{
        AVI_RGB_FORMAT = 0x00,
        AVI_YCBCR_422,
        AVI_YCBCR_444
};

enum AVI_IFM_AFI_PRESENT
{
        AFI_PRESENT_NO_DATA = 0x00,
        AFI_PRESENT_VALID
};

enum AVI_IFM_BAR_INFO
{
        BAR_INFO_NOT_VALID = 0x00,
        VERT_BAR_INFO_VALID,
        HORIZ_BAR_INFO_VALID,
        VERT_HORIZ_BAR_INFO_VALID,
};

enum AVI_IFM_SCAN_INFO
{
        SCAN_INFO_NO_DATA = 0x00,
        SCAN_INFO_OVERSCANED,
        SCAN_INFO_UNDERSCANNED
};

enum AVI_IFM_COLORIMETRY
{
        COLORIMETRY_NO_DATA = 0x00,
        COLORIMETRY_SMPTE_170M,                 //481i(p),576i(p),240p and 288p
        COLORIMETRY_ITU709                              //1080i(p) AND 720p
};

enum AVI_IFM_ASPECT_RATIO
{
        AR_NO_DATA = 0x00,
        AR_4_3,
        AR_16_9
};

enum AVI_IFM_AFD
{
        AFD_16_9_TOP                            = 0x02,
        AFD_14_9_TOP                            = 0x03,
        AFD_GREATER_16_9_CENTER = 0x04,
        AFD_AS_CODE_FRAME               = 0x08,
        AFD_4_3_CENTER                  = 0x09,
        AFD_16_9_CENTER                 = 0x0A,
        AFD_14_9_CENTER                 = 0x0B,
        AFD_4_3                                 = 0x0D,
        AFD_16_9_PROTECT_14_9   = 0x0E,
        AFD_16_9_PROTECT_4_3            = 0x0F
};

enum AVI_IFM_RGB_QUANT_RANGE
{
        DEFAULT = 0x00,
        LIMITED,
        FULL
};

enum AVI_IFM_SCALING
{
        NO_KNOWN_SCALING = 0x00,
        HORIZ_SCALED,
        VERT_SCALED,
        HORIZ_VERT_SCALED
};

struct AVI_PAYLOAD
{
        UINT8 check_sum;
        // Byte 1
        UINT8 scan_infor                        : 2;            // S1,S0: Scan Information
        UINT8 bar_infor                         : 2;            // B1,B0: Bar Info
        UINT8 afi_present                       : 1;            // A0: Active Format Information Present
        UINT8 format                            : 2;            // Y1, Y0: RGB or YCbCr
        UINT8 reserved_1                        : 1;            // F7 Future Use, All zeros
        // Byte 2
        UINT8 afd                                       : 4;            // R3, R2, R1, R0: Active Format Aspect Ratio
        UINT8 aspect_ratio                      : 2;            // M1, M0: Picture Aspect Ratio
        UINT8 colorimetry                       : 2;            // C1, C0: Colormetry
        // Byte 3
        UINT8 non_uniform_scale         : 2;            // SC1, SC0: Non-Uniform Picture Scaling
        UINT8 reserved_2                        : 6;
        // Byte 4
        UINT8 video_identfy_code        : 7;
        UINT8 reserved_3                        : 1;
        // Byte 5
        UINT8 pixel_repeat                      : 4;
        UINT8 reserved_4                        : 4;

        UINT8 ln_etb_lower;                             //line number of end of top bar (lower 8 bits)
        UINT8 ln_etb_upper;                             //line number of end of top bar (upper 8 bits)
        UINT8 ln_sbb_lower;                             //line number of start of bottom bar (lower 8 bits)
        UINT8 ln_sbb_upper;                             //line number of start of bottom bar (upper 8 bits)

        UINT8 pn_elb_lower;                             //pixel number of end of left bar (lower 8 bits)
        UINT8 pn_elb_upper;                             //pixel number of end of left bar (upper 8 bits)
        UINT8 pn_srb_lower;                             //pixel number of start of right bar (lower 8 bits)
        UINT8 pn_srb_upper;                             //pixel number of start of right bar (upper 8 bits)
};


struct ifm_pkt_header
{
        UINT8 InfoFram_type;
        UINT8 InfoFram_version;
        UINT8 InfoFram_length;
};

struct ifm_packet
{
        struct ifm_pkt_header header;
        UINT8 *payload;
};



#define US_TICKS    (216000000 / 2000000)


/* get video identification code for AVI InfoFrame. Pls ref. CEA-861-B p.61 */
static UINT8 get_ifm_video_identy_code ( enum TVSystem tv_mode, BOOL scan_mode, enum TVMode output_aspect_ratio )
{
        UINT8 vic = 0x00;
        if ( PAL_M == tv_mode || NTSC == tv_mode || NTSC_443 == tv_mode )
        {
                if ( 0 == scan_mode )
                {
                        if ( TV_4_3 == output_aspect_ratio )
                                vic = 6;
                        else
                                vic = 7;
                }
                else
                {
                        if ( TV_4_3 == output_aspect_ratio )
                                vic = 2;
                        else
                                vic = 3;
                }
        }
        else if ( PAL == tv_mode || PAL_N == tv_mode || PAL_60 == tv_mode )
        {
                if ( 0 == scan_mode )
                {
                        if ( TV_4_3 == output_aspect_ratio )
                                vic = 21;
                        else
                                vic = 22;
                }
                else
                {
                        if ( TV_4_3 == output_aspect_ratio )
                                vic = 17;
                        else
                                vic = 18;
                }
        }
        else if ( LINE_720_25 == tv_mode )
        {
                vic = 19;
        }
        else if ( LINE_720_30 == tv_mode )
        {
                vic = 4;
        }
        else if ( LINE_1080_25 == tv_mode )
        {
                vic = 20;
        }
        else if ( LINE_1080_30 == tv_mode )
        {
                vic = 5;
        }
        else
                vic = 0;

        return vic;
}

/* calculate CheckSum value of InfoFrame Packet */
static INT8 calculate_check_sum ( struct ifm_packet *packet )
{
        INT16 check_sum = 0;
        UINT8 length = packet->header.InfoFram_length + 1;
        UINT8 i;
        check_sum = packet->header.InfoFram_type + packet->header.InfoFram_version + packet->header.InfoFram_length;
        for ( i = 1; i < length; i++ )
        {
                check_sum += packet->payload[i];
        }
        return -check_sum;
}

static void hdmi_set_T_sel_type ( BOOL bSelect )
{
        UINT8 buf = hdmi_ReadByte ( R_CFG5 );
        hdmi_WriteByte ( R_CFG5, ( ( bSelect == TRUE ) ? ( buf | B_T_SEL ) : ( buf & ( ~B_T_SEL ) ) ) );
}
static void hdmi_set_pll_sel ( UINT8 pll_sel )
{
        UINT8 buf = hdmi_ReadByte ( R_CFG5 );
        hdmi_WriteByte ( R_CFG5, ( buf & ~ ( B_PLL_SEL0 | B_PLL_SEL1 ) ) | ( pll_sel & ( B_PLL_SEL0 | B_PLL_SEL1 ) ) );
        UINT8 res_buf = hdmi_ReadByte ( R_CFG6 );
        hdmi_WriteByte ( R_CFG6, ( res_buf & ~B_PLL_SEL2 ) | ( ( pll_sel << 2 ) & B_PLL_SEL2 ) );
}
static void hdmi_set_emp_en ( UINT8 emp_en )
{
        UINT8 buf = hdmi_ReadByte ( R_CFG6 );
        hdmi_WriteByte ( R_CFG6, ( buf & ~ ( B_EMP_EN ) ) | ( ( emp_en << 1 ) & ( B_EMP_EN ) ) );
}
void hdmi_show_on_bootloader ( struct de2Hdmi_video_infor *video_info )
{
        UINT8 i, buf;
        struct ifm_packet avi_infoFrame;
        struct AVI_PAYLOAD avi_payload;

        avi_infoFrame.header.InfoFram_type = 0x82;
        avi_infoFrame.header.InfoFram_version = 0x02;
        avi_infoFrame.header.InfoFram_length = 0x0D;
        avi_infoFrame.payload = &avi_payload;
        MEMSET ( avi_infoFrame.payload, 0, sizeof ( UINT8 ) * ( 0x0D + 1 ) );

        /**********************************************************
         * prepare AVI InfoFrame data according to vp video_info
         *********************************************************/

        //prepare the format of InfoFrame
        switch ( video_info->format )
        {
                case RGB_MODE1:
                case RGB_MODE2:
                        avi_payload.format = AVI_RGB_FORMAT;
                        break;
                case YCBCR_444:
                        avi_payload.format = AVI_YCBCR_444;
                        break;
                case YCBCR_422:
                        avi_payload.format = AVI_YCBCR_422;
                        break;
        }
        avi_payload.bar_infor = BAR_INFO_NOT_VALID;
        avi_payload.scan_infor = SCAN_INFO_OVERSCANED;

        //prepare the colorimetry of InfoFrame
        switch ( video_info->tv_mode )
        {
                case LINE_720_25:
                case LINE_720_30:
                case LINE_1080_25:
                case LINE_1080_30:
                        avi_payload.colorimetry = COLORIMETRY_ITU709;
                        break;
                case PAL:
                case PAL_N:
                case PAL_60:
                case PAL_M:
                case NTSC:
                case NTSC_443:
                        avi_payload.colorimetry = COLORIMETRY_SMPTE_170M;
                        break;
                default:
                        avi_payload.colorimetry = COLORIMETRY_NO_DATA;
                        break;
        }

        avi_payload.afi_present = AFI_PRESENT_NO_DATA;
        avi_payload.afd = AFD_AS_CODE_FRAME;

        //prepare the picture aspect ratio of InfoFrame
        switch ( video_info->output_aspect_ratio )
        {
                case TV_4_3:
                        avi_payload.aspect_ratio = AR_4_3;
                        break;
                case TV_16_9:
                        avi_payload.aspect_ratio = AR_16_9;
                        break;
                default:
                        avi_payload.aspect_ratio = AR_NO_DATA;
                        break;
        }

        avi_payload.non_uniform_scale = NO_KNOWN_SCALING;

        //prepare the video identification codes of InfoFrame
        avi_payload.video_identfy_code = get_ifm_video_identy_code ( video_info->tv_mode, video_info->scan_mode, video_info->output_aspect_ratio );

        //prepare the pixel repetition of InfoFrame
        if ( 6 == avi_payload.video_identfy_code || 7 == avi_payload.video_identfy_code || 21 == avi_payload.video_identfy_code || 22 == avi_payload.video_identfy_code )
                avi_payload.pixel_repeat = 0x01;
        else
                avi_payload.pixel_repeat = 0x00;

        avi_payload.ln_etb_lower = 0x00;
        avi_payload.ln_etb_upper = 0x00;
        avi_payload.ln_sbb_lower = 0x00;
        avi_payload.ln_sbb_upper = 0x00;
        avi_payload.pn_elb_lower = 0x00;
        avi_payload.pn_elb_upper = 0x00;
        avi_payload.pn_srb_lower = 0x00;
        avi_payload.pn_srb_upper = 0x00;
        avi_payload.check_sum = calculate_check_sum ( &avi_infoFrame );

        /**********************************************************
         * HDMI Hardware initial
         *********************************************************/
        // global reset hdmi module

        * ( volatile UINT32 * ) ( 0xb8000080 ) = ( * ( volatile UINT32 * ) ( 0xb8000080 ) ) | 0x00002000;
        osal_delay ( 5 );
        * ( volatile UINT32 * ) ( 0xb8000080 ) = ( * ( volatile UINT32 * ) ( 0xb8000080 ) ) & 0xffffdfff;
        osal_delay ( 5 );
        * ( ( volatile UINT32 * ) 0xB8000400 ) = ( * ( ( volatile UINT32 * ) 0xB8000400 ) & ( 0x00001F00 ) ) | 0x00000300;
        if ((sys_ic_get_chip_id() != ALI_S3811) && (sys_ic_get_chip_id() != ALI_C3701))
        {
                hdmi_set_T_sel_type ( TRUE );
        }
        hdmi_set_pll_sel ( 0x02 );
        hdmi_set_emp_en ( 0x01 );

        // Fill bist value
        for ( i = 0; i < 4; i++ )
        {
                hdmi_WriteByte ( R_BIST + i, ( UINT8 ) ( ( 0x354d5354 >> ( i * 8 ) ) & 0x000000FF ) );
        }

        // SRST & Power down HDMI PHY (Turn off TMDS signal)
        hdmi_WriteByte ( R_CTRL, ( hdmi_ReadByte ( R_CTRL ) | B_SRST | B_PDB ) );
        osal_delay ( 5 );

        // Sel Phy Clk source & HDMI Mode
        if ((sys_ic_get_chip_id() != ALI_S3811) && (sys_ic_get_chip_id() != ALI_C3701))  //when 3811,3701,should set ck_sel to 0(default,else set to 1),by ze
                hdmi_WriteByte ( R_CFG1, ( hdmi_ReadByte ( R_CFG1 ) | B_CK_SEL ) & ( ~B_DVI ) );

        // Set Pre Driver Control
        hdmi_WriteByte ( R_DRV_CTL + 1, ( ( 0x0777 & 0xFF00 ) >> 8 ) );
        hdmi_WriteByte ( R_DRV_CTL, ( 0x0777 & 0x00FF ) );

        // Power On PHY
        hdmi_WriteByte ( R_CTRL, ( hdmi_ReadByte ( R_CTRL ) & ( ~B_PDB ) ) );
        osal_delay ( 5 );

        // Enable Info frame Packet
        hdmi_WriteByte ( R_CTRL, hdmi_ReadByte ( R_CTRL ) | ( B_GENERIC_EN | B_AVI_EN ) );



        /**********************************************************
         * Transmit AVI Info Frame
         *********************************************************/

        // Write Info Frame Header
        hdmi_WriteByte ( R_IFM_TYPE, avi_infoFrame.header.InfoFram_type );
        osal_delay ( 5 );
        hdmi_WriteByte ( R_IFM_VER, avi_infoFrame.header.InfoFram_version );
        osal_delay ( 5 );
        hdmi_WriteByte ( R_IFM_LENG, avi_infoFrame.header.InfoFram_length );
        osal_delay ( 5 );

        // Write Info Frame Content
        for ( i = 0; i < avi_infoFrame.header.InfoFram_length + 1; i++ )
        {
                hdmi_WriteByte ( R_IFM_PORT, avi_infoFrame.payload[i] );
                osal_delay ( 5 );
        }

        hdmi_WriteByte ( R_IFM_TYPE, 0x00 );
        osal_delay ( 5 );

        // Modify I/O sync module to fix asynchronous interface
        // logic bug (NCTS/AV info frame packeage Error, add a dummy address)
        hdmi_WriteByte ( R_DUMY_ADDR, 0x00 );

        //hdmi_WriteByte(R_CTRL, (hdmi_ReadByte(R_CTRL)|B_AV_MUTE) );
        //osal_delay(5000);
        //hdmi_WriteByte(R_CTRL, (hdmi_ReadByte(R_CTRL)&(~B_AV_MUTE)) );

        /**********************************************************
         * Turn On HDMI Video output
         *********************************************************/
        hdmi_WriteByte ( R_CTRL, ( hdmi_ReadByte ( R_CTRL ) & ( ~B_SRST ) ) );
}
