#ifndef __ADF_SND_H
#define __ADF_SND_H



/*! @addtogroup Devicedriver
 *  @{
 */
 
/*! @addtogroup ALiAudio
 *  @{
*/


/*! @struct spec_param
@brief A structure defines spectrum parameters
*/
typedef struct
{
    void (*spec_call_back) (INT32 *);   //!< The spectrum callbak
    UINT32 collumn_num;                 //!< The column number
} spec_param;



/*! @struct spec_step_table
@brief A structure defines spectrum step table.
*/
typedef struct
{
    UINT32 column_num;  //!< The column number
    UINT8 *ptr_table;   //!< The spectrum step table address
} spec_step_table;



/*! @struct snd_sync_param
@brief A structure defines synchronization parameters for sound device.
*/
typedef struct
{
    UINT32 drop_threshold;          //!< The drop frame threshold
    UINT32 wait_threshold;          //!< The wait frame threshold
    UINT32 delay_video_sthreshold;  //!< The delay video frame threshold
    UINT32 hold_threshold;          //!< The hold frame threshold
    UINT32 dma_remain_threshold;    //!< The DMA remain frame threshold
} snd_sync_param;



/*! @struct snd_spdif_scms
@brief A structure defines SPDIF output information for for CI+ protect.
*/
struct snd_spdif_scms
{
    UINT8 copyright:1;      //!< Stream copyright
    UINT8 reserved:7;       //!< Reserved bits
    UINT8 l_bit:1;          //!< Linear PCM smaples flag or Non-PCM
    UINT8 category_code:7;  //!< Category code
    UINT16 reserved16;      //!< Reserved bits
};



#define CODEC_I2S (0x0<<1)      //!< I2S codec format 
#define CODEC_LEFT (0x1<<1)     //!< Left codec format
#define CODEC_RIGHT (0x2<<1)    //!< Right codec format


#define STC_DELAY                   0x400       //!< STC delay threshold
#define MUTE_BY_GPIO                0           //!< Control mute function by GPIO
#define MUTE_BY_SCART               1           //!< Control mute function by SCART

#define SND_STATE_DETACH            0           //!< Sound Device is in DETTACH state
#define SND_STATE_ATTACH            1           //!< Sound Device is in ATTACH state
#define SND_STATE_IDLE              2           //!< Sound Device is in IDLE state
#define SND_STATE_PLAY              4           //!< Sound Device is in PLAY state
#define SND_STATE_PAUSE             8           //!< Sound Device is in PAUSE state

#define SND_SUB_STATE_BUSY          1           //!< Sound Device is in BUSY sub state
#define SND_SUB_STATE_NO_DATA       2           //!< Sound Device is in no DATA sub state
#define SND_SUB_STATE_NO_BUFF       4           //!< Sound Device is in no Buffer sub state



/* define the device IO control code for sound */
#define SND_IO                                  0x0000000F   //!< Sound output device ioctl command base
#define IS_SND_RUNNING                          (SND_IO + 1) //!< Check wheather sound device is in PLAY state
#define IS_SND_MUTE                             (SND_IO + 2) //!< Check wheather sound device is mute
#define SND_CC_MUTE                             (SND_IO + 3) //!< Set volume to zero and close internal DAC
#define SND_CC_MUTE_RESUME                      (SND_IO + 4) //!< Resume vlume to last setting and open internal DAC
#define SND_SET_FADE_SPEED                      (SND_IO + 5) //!< Set volume fade in and out speed
#define IS_PCM_EMPTY                            (SND_IO + 6) //!< Check wheather the I2SO DMA buffer is empty 
#define SND_PAUSE_MUTE                          (SND_IO + 7) //!< Set volume to zero when the sound device is not mute
#define SND_SPO_ONOFF                           (SND_IO + 8) //!< Open or close the SPO/DDP_SPO interface
#define SND_REQ_REM_DATA                        (SND_IO + 9) //!< Get the remain pcm data size still not play yet
#define SND_SPECTRUM_START                      (SND_IO + 10) //!< Start to spectrum play
#define SND_SPECTRUM_STOP                       (SND_IO + 11) //!< Stop to spectrum play
#define SND_SPECTRUM_CLEAR                      (SND_IO + 12) //!< Clear spectrum play data
#define SND_BYPASS_VCR                          (SND_IO + 13) //!< Set bypass VCR
#define FORCE_SPDIF_TYPE                        (SND_IO + 14) //!< Set the spdif output type, strongly recommend call this command in channel change task.
#define SND_DAC_MUTE                            (SND_IO + 15) //!< Do not use any more
#define SND_CHK_SPDIF_TYPE                      (SND_IO + 16) //!< Check SPDIF output type
#define SND_CHK_DAC_PREC                        (SND_IO + 17) //!< Do not use any more
#define SND_CHK_PCM_BUF_DEPTH                   (SND_IO + 18) //!< Get the pcm DMA buffer size in frame unit
#define SND_POST_PROCESS_0                      (SND_IO + 19) //!< Do not use any more
#define SND_SPECIAL_MUTE_REG                    (SND_IO + 20) //!< Do not use any more
#define STEREO_FUN_ON                           (SND_IO + 21) //!< Do not use any more
#define SND_REQ_REM_PCM_DATA                    (SND_IO + 22) //!< Get the remain pcm data size still not play yet, equal to SND_REQ_REM_DATA
#define SND_SPECTRUM_STEP_TABLE                 (SND_IO + 23) //!< Set spectrum step table
#define SND_SPECTRUM_VOL_INDEPEND               (SND_IO + 24) //!< Set spectrum volume independent
#define SND_SPECTRUM_CAL_COUNTER                (SND_IO + 25) //!< Set spectrum caculate counter
#define SND_SET_SYNC_DELAY                      (SND_IO + 26) //!< Set audio sync delay time
#define SND_REQ_REM_PCM_DURA                    (SND_IO + 27) //!< Get the remain data duration playing time
#define SND_SET_SYNC_LEVEL                      (SND_IO + 28) //!< Set audio sync level
#define SND_GET_SPDIF_TYPE                      (SND_IO + 29) //!< Get the SPDIF output type
#define SND_SET_BS_OUTPUT_SRC                   (SND_IO + 30) //!< Set DDP_SPO(HDMI) output source
#define SND_SET_MUTE_TH                         (SND_IO + 31) //!< Do not use any more
#define SND_GET_MUTE_TH                         (SND_IO + 32) //!< Do not use any more
#define SND_SET_SPDIF_SCMS                      (SND_IO + 33) //!< struct snd_spdif_scms *
#define SND_GET_SAMPLES_REMAIN                  (SND_IO + 34) //!< Get the remain PCM samples size still not output yet
#define SND_SECOND_DECA_ENABLE                  (SND_IO + 35) //!< Enable second DECA for audio description
#define SND_SET_DESC_VOLUME_OFFSET              (SND_IO + 36) //!< Set audio description voume offset
#define SND_GET_TONE_STATUS                     (SND_IO + 37) //!< Get tone volume playing status
#define SND_DO_DDP_CERTIFICATION                (SND_IO + 38) //!< Enable sound device do DDP certifictation function
#define SND_AUTO_RESUME                         (SND_IO + 39) //!< Enable sound device auto resume function when error occurs.
#define SND_SET_SYNC_PARAM                      (SND_IO + 40) //!< Set synchronization parameters
#define SND_RESET_DMA_BUF                       (SND_IO + 41) //!< Reset sound dma buffer to drop some frames, Only used for SEE side, Please do not use it.
#define SND_I2S_OUT                             (SND_IO + 42) //!< Enable I2S output audio
#define SND_HDMI_OUT                            (SND_IO + 43) //!< Enable HDMI output audio
#define SND_SPDIF_OUT                           (SND_IO + 44) //!< Enable SPDIF output audio
#define SND_SET_FRAME_SHOW_PTS_CALLBACK         (SND_IO + 45) //!< Do not use any more
#define SND_MPEG_M8DB_ENABLE                    (SND_IO + 46) //!< Enable MPEG M8DB ajustment for pcm data. Only used for SEE side, Please do not use it.
#define SND_GET_SYNC_PARAM                      (SND_IO + 47) //!< Send sync parameters to AVSYNC module. Only used for SEE side, Please do not use it.
#define SND_ENABLE_DROP_FRAME                   (SND_IO + 48) //!< Only used for SEE side, Please do not use it.
#define SND_REG_GET_SYNC_FLAG_CB                (SND_IO + 49) //!< Get AV synchronization flag control block from avsync module. Only used for SEE side, Please do not use it.
#define SND_SET_HW_HOLD_THRESHOLD               (SND_IO + 50) //!< Hardware hold DMA threshold by avsync module, Only used for SEE side, Please do not use it.
#define SND_GET_RESET_PARAM                     (SND_IO + 51) //!< Only used for SEE side, Please do not use it.
#define SND_SET_RESET_PARAM                     (SND_IO + 52) //!< Only used for SEE side, Please do not use it.
#define SND_GET_STATUS                          (SND_IO + 53) //!< Get sound device status
#define SND_GET_RAW_PTS                         (SND_IO + 54) //!< Get raw PTS
#define SND_DMX_SET_VIDEO_TYPE                  (SND_IO + 55) //!< Only used for SEE side, Please do not use it.
#define SND_IO_PAUSE_SND                        (SND_IO + 56) //!< Pause sound device, Only used for SEE side, Please do not use it.
#define SND_IO_RESUME_SND                       (SND_IO + 57) //!< Resume sound device, Only used for SEE side, Please do not use it.

#define SND_HDMI_ENABLE                         (SND_IO + 58) //!< Only used for SEE side, Please do not use it.
#define SND_RESTART                             (SND_IO + 59) //!< Only used for SEE side, Please do not use it.
#define SND_STOP_IMMD                           (SND_IO + 60) //!< Only used for SEE side, Please do not use it.
#define SND_DO_DDP_CERTIFICATION_EX             (SND_IO + 61) //!< Only used for SEE side, Please do not use it.
#define SND_BUF_DATA_REMAIN_LEN                 (SND_IO + 62) //!< Only used for SEE side, Please do not use it.

#define SND_ONLY_SET_SPDIF_DELAY_TIME           (SND_IO + 70) //!< Set SPDIF delay time (0ms-250ms)
#define SND_HDMI_CONFIG_SPO_CLK                 (SND_IO + 71) //!< HDMI config DDP_SPO clock to SPO clock

#define SND_ADV_IO                              (SND_IO + 0x200) //!< Sound sub ioctl command base
#define SND_BASS_TYPE                           (SND_ADV_IO + 1) //!< Do not use any more
#define SND_REG_HDMI_CB                         (SND_ADV_IO + 2) //!< Set the HDMI callback for sound device
#define SND_PCM_DUMP_ON                         (SND_ADV_IO + 3) //!< Do not use any more
#define SND_PCM_DUMP_OFF                        (SND_ADV_IO + 4) //!< Do not use any more

//added by kinson for kalaok
#define SND_I2SIN_MIC0_GET_ENABLE        (SND_ADV_IO + 20)
#define SND_I2SIN_MIC0_SET_ENABLE        (SND_ADV_IO + 21)
#define SND_I2SIN_MIC0_GET_VOLUME        (SND_ADV_IO + 22)
#define SND_I2SIN_MIC0_SET_VOLUME        (SND_ADV_IO + 23)
#define SND_I2SIN_MIC0_START             (SND_ADV_IO + 24)
#define SND_I2SIN_MIC0_STOP              (SND_ADV_IO + 25)

#define SND_I2SIN_MIC1_GET_ENABLE        (SND_ADV_IO + 26)
#define SND_I2SIN_MIC1_SET_ENABLE        (SND_ADV_IO + 27)
#define SND_I2SIN_MIC1_GET_VOLUME        (SND_ADV_IO + 28)
#define SND_I2SIN_MIC1_SET_VOLUME        (SND_ADV_IO + 29)
#define SND_I2SIN_MIC1_START             (SND_ADV_IO + 30)
#define SND_I2SIN_MIC1_STOP              (SND_ADV_IO + 31)








/*****************************************************/

// SPDIF raw data coding type
#define SPDO_SRC_FLR                0x00 //!< Do not use any more
#define SPDO_SRC_SLR                0x01 //!< Do not use any more
#define SPDO_SRC_CSW                0x02 //!< Do not use any more
#define SPDO_SRC_DMLR               0x03 //!< Do not use any more
#define SPDO_SRC_EXLR               0x04 //!< Do not use any more
#define SPDO_SRC_BUF                0x07 //!< Do not use any more
#define SPDO_SRC_LFEC               0x01 //!< Do not use any more


//bass_type
#define BASS_DISABLE                0x00 //!< Do not use any more
#define BASS_CON0                   0x01 //!< Do not use any more
#define BASS_CON1                   0x02 //!< Do not use any more
#define BASS_CON2_NSUB              0x03 //!< Do not use any more
#define BASS_CON2_WSUB              0x04 //!< Do not use any more
#define BASS_ALT_CON2               0x05 //!< Do not use any more
#define BASS_CON3_NSUB              0x06 //!< Do not use any more
#define BASS_CON3_WSUB              0x07 //!< Do not use any more
#define BASS_SIMP_NSUB              0x08 //!< Do not use any more
#define BASS_SIMP_WSUB              0x09 //!< Do not use any more



/*! @enum SndToneStatus
@brief Do not use any more.
*/
enum SndToneStatus
{
    SND_STREAM_STATUS = 0, //!< None
    SND_TONE_STATUS,       //!< Left channel
};



/*! @enum SndChannel
@brief Do not use any more.
*/
enum SndChannel
{
    SND_CH_NONE = 0,    //!< None
    SND_CH_L,           //!< Left channel
    SND_CH_R,           //!< Right channel
    SND_CH_LS,          //!< Left surround channel
    SND_CH_RS,          //!< Right surround channel
    SND_CH_C,           //!< Center channel
    SND_CH_LFE,         //!< Low frequency effect channel
    SND_CH_DML,         //!< downmix L channel
    SND_CH_DMR          //!< downmix R channel
};



/*! @enum SndDownMixMode
@brief Do not use any more.
*/
enum SndDownMixChannel
{
    SND_DOWNMIXCHANNEL_DM, //!< Do not use any more
    SND_DOWNMIXCHANNEL_LR  //!< Do not use any more
};



/*! @enum SndDownMixMode
@brief Do not use any more.
*/
enum SndDownMixMode
{
    SND_DOWNMIXMODE_51 = 1, //!< Do not use any more
    SND_DOWNMIXMODE_LORO,   //!< Do not use any more
    SND_DOWNMIXMODE_LTRT,   //!< Do not use any more
    SND_DOWNMIXMODE_VIR     //!< Do not use any more
};



/*! @enum SndDescOutputChannel
@brief Do not use any more.
*/
enum SndDescOutputChannel
{
    SND_FORWARD_CH = 1, //!< Do not use any more
    SND_SURROUND_CH,    //!< Do not use any more
    SND_LFE_C_CH,       //!< Do not use any more
    SND_DOWNMIX_CH      //!< Do not use any more
};



/*! @enum SndEqualizer
@brief Do not use any more.
*/
enum SndEqualizer
{
    SND_EQ_NONE = 0,        //!< Disable equalizer mode.
    SND_EQ_CLASSIC,         //!< Classsic mode.
    SND_EQ_ROCK,            //!< Rock mode.
    SND_EQ_JAZZ,            //!< Jazz mode.
    SND_EQ_POP,             //!< Pop mode
    SND_EQ_BASS,            //!< Bass mode.
    SND_EQ_USER             //!< User define mode.
};



/*! @enum SndReverb
@brief Do not use any more.
*/
enum SndReverb
{
    SND_REVERB_OFF = 0, //!< Do not use any more
    SND_REVERB_CHURCH,  //!< Do not use any more
    SND_REVERB_CINEMA,  //!< Do not use any more
    SND_REVERB_CONCERT, //!< Do not use any more
    SND_REVERB_HALL,    //!< Do not use any more
    SND_REVERB_LIVE,    //!< Do not use any more
    SND_REVERB_ROOM,    //!< Do not use any more
    SND_REVERB_STADIUM, //!< Do not use any more
    SND_REVERB_STANDARD //!< Do not use any more
};



/*! @enum SndSpeakerSize
@brief Do not use any more.
*/
enum SndSpeakerSize
{
    SND_SPEAKER_SIZE_OFF= 0,    //!< Do not use any more
    SND_SPEAKER_SIZE_ON,        //!< Do not use any more
    SND_SPEAKER_SIZE_SMALL,     //!< Do not use any more
    SND_SPEAKER_SIZE_BIG        //!< Do not use any more
};



/*! @enum SndDRC
@brief Do not use any more
*/
enum SndDRC
{
    SND_DRC_OFF = 0,    //!< Do not use any more
    SND_DRC_CUSTOM_A,   //!< Do not use any more
    SND_DRC_CUSTOM_D,   //!< Do not use any more
    SND_DRC_LINE_OUT,   //!< Do not use any more
    SND_DRC_RF_REMOD    //!< Do not use any more
};



/*! @enum ASndOutMode
@brief Do not use any more.
*/
enum ASndOutMode
{
    SND_OUT_GEN = 0,    //!< Do not use any more
    SND_OUT_DIG,        //!< Do not use any more
    SND_OUT_DIGGEN      //!< Do not use any more
};



/*! @enum SndSubBlock
@brief A enum defines the sound device sub block.
*/
enum SndSubBlock
{
    SND_SUB_PP = 0x01,          //!< Audio post-process
    SND_SUB_IN = 0x02,          //!< General audio input interface
    SND_SUB_OUT = 0x04,         //!< General audio output interface
    SND_SUB_MIC0 = 0x08,        //!< Micro phone 0 input interface
    SND_SUB_MIC1 = 0x10,        //!< Micro phone 1 input interface
    SND_SUB_SPDIFIN = 0x20,     //!< SPDIF input interface
    SND_SUB_SPDIFOUT = 0x40,    //!< SPDIF output interface
    SND_SUB_SPDIFOUT_DDP = 0x80,//!< DDP_SPDIF output interface (for HDMI)
    SND_SUB_ALL = 0xff          //!< All IO enabled
};



/*! @enum SndDupChannel
@brief A enum defines the output channel mode.
*/
enum SndDupChannel
{
    SND_DUP_NONE,   //!< Output stereo 
    SND_DUP_L,      //!< Output left channel data
    SND_DUP_R,      //!< Output right channel data
    SND_DUP_MONO    //!< Output mono
};



/*! @enum SndSpoOutputSrcType
@brief A enum defines the SPDIF output type.
*/
enum ASndOutSpdifType
{
    SND_OUT_SPDIF_INVALID = -1, //!< Invalid type
    SND_OUT_SPDIF_PCM = 0,      //!< SPDIF output pcm data
    SND_OUT_SPDIF_BS = 1,       //!< SPDIF output bit stream
    SND_OUT_SPDIF_FORCE_DD = 2  //!< Do not use any more
};



/*! @enum SndSpoOutputSrcType
@brief A enum defines the HDMI output source type.
*/
enum SndSpoOutputSrcType
{
    SND_OUT_SRC_DDPSPO = 0, //!< HDMI output from DDP SPO DMA
    SND_OUT_SRC_SPO = 1     //!< HDMI output from SPO DMA
};



/*! @enum EQ_TYPE
@brief Do not use any more.
*/
enum EQ_TYPE
{
    EQ_SLIGHT     = 0,  //!< Do not use any more
    EQ_CLASSIC    = 1,  //!< Do not use any more
    EQ_ELECTRONIC = 2,  //!< Do not use any more
    EQ_DANCE      = 3,  //!< Do not use any more
    EQ_LIVE       = 4,  //!< Do not use any more
    EQ_POP        = 5,  //!< Do not use any more
    EQ_ROCK       = 6   //!< Do not use any more
};



/*! @struct snd_output_cfg
@brief A structure defines configuration of the sound device output.
*/
struct snd_output_cfg
{
    UINT8 mute_num;                     //!< Mute circuit gpio number
    UINT8 mute_polar;                   //!< The polarity which will cause circuit mute 
    UINT8 dac_precision;                //!< Internal DAC precision: 24bit or 16bit 
    UINT8 dac_format;                   //!< Internal DAC format: CODEC_I2S (0x0<<1), CODEC_LEFT (0x1<<1), CODEC_RIGHT (0x2<<1)
    UINT8 is_ext_dac;                   //!< The flag indicate wheather is embedded dac.	
    UINT8 reserved8;                    //!< Reserved bits
    UINT16 gpio_mute_circuit:1;         //!< FALSE: no mute circuit; TRUE: exists mute circuit controlled by GPIO
    UINT16 ext_mute_mode:2;             //!< External mute mode
    UINT16 enable_hw_accelerator:1;     //!< Enable hardware accelerator flag, FALSE: do not enable M3202 audio HW accelerator, TRUE: Enable M3202 audio HW accelerator;
    UINT8 chip_type_config:1;           //!< The chip type of ALi, 1: QFP; 0: BGA.
    UINT16 reserved:11;                 //!< Reserved bits
};



/*! @struct pcm_output
@brief A structure defines the output pcm data information.
*/
struct pcm_output
{
    UINT32 ch_num;          //!< The channel number of PCM frame
    UINT32 ch_mod;          //!< The output channel mode
    UINT32 samp_num;        //!< The sample number of PCM frame
    UINT32 sample_rata_id;  //!< The sample rate id of PCM frame
    UINT32 inmode;          //!< The inmode of PCM frame
    UINT32 *ch_left;        //!< The buffer address of left channel
    UINT32 *ch_right;       //!< The buffer address of right channel
    UINT32 *ch_sl;          //!< The buffer address of left surround channel
    UINT32 *ch_sr;          //!< The buffer address of right surround channel
    UINT32 *ch_c;           //!< The buffer address of center channel
    UINT32 *ch_lfe;         //!< The buffer address of low frequency effects channel
    UINT32 *ch_dl;          //!< The buffer address of direct left channel
    UINT32 *ch_dr;          //!< The buffer address of direct right channel
    UINT32 *ch_left_m8db;   //!< The buffer address of left channel with -31db adjustment for MPEG SPDIF output in BS out mode
    UINT32 *ch_right_m8db;  //!< The buffer address of right channel with -31db adjustment for MPEG SPDIF output in BS out mode
    UINT32 *ch_sl_m8db;     //!< The buffer address of left surround channel with -31db adjustment for MPEG SPDIF output in BS out mode
    UINT32 *ch_sr_m8db;     //!< The buffer address of right surround channel with -31db adjustment for MPEG SPDIF output in BS out mode
    UINT32 *ch_c_m8db;      //!< The buffer address of center channel with -31db adjustment for MPEG SPDIF output in BS  out mode
    UINT32 *ch_lfe_m8db;    //!< The buffer address of low frequency effects channel with -31db adjustment for MPEG SPDIF output in BS out mode
    UINT32 *ch_dl_m8db;     //!< The buffer address of direct left channel with -31db adjustment for MPEG SPDIF output in BS out mode
    UINT32 *ch_dr_m8db;     //!< The buffer address of direct right channel with -31db adjustment for MPEG SPDIF output in BS out mode
    UINT8  *raw_data_start; //!< The spo buffer address of bit stream data
    UINT32 raw_data_len;    //!< The length of the bit stream data
    UINT32 iec_pc;          //!< The PC value in AC3 stream header

    UINT8  *raw_data_ddp_start; //!< The ddp_spo buffer address of bit stream data
    UINT32 raw_data_ddp_len;    //!< The length of the bit stream data
    UINT8  iec_pc_ddp;          //!< The PC value in EAC3 stream header
};



/*! @struct snd_feature_config
@brief A structure defines the sound device feature to be configured.
*/
struct snd_feature_config
{
    struct snd_output_cfg output_config;    //!< Sound device output configuration
    UINT8 support_spdif_mute;               //!< Support SPDIF interface mute flag
    UINT8 swap_lr_channel;                  //!< Support left and right channel swap flag
    UINT8 conti_clk_while_ch_chg;           //!< Support continue clock while channel change for Tone voice
    UINT8 support_desc;                     //!< Support audio narraotr channel output flag
};



/*! @struct snd_dev_status
@brief A structure defines the sound device status.
*/
struct snd_dev_status
{
    UINT8 flags;            //!< Sound device flags
    UINT32 volume;          //!< The volume sound device output
    UINT32 in_mute;         //!< The flag wheather sound device mute
    UINT8 spdif_out;        //!< The flag wheather spdif data valid
    UINT8 trackmode;        //!< The channel mode, refer to enum SndDupChannel
    
    UINT32 samp_rate;       //!< The sample rate of input audio stream
    UINT32 samp_num;        //!< The sample number of input audio stream
    UINT32 ch_num;          //!< The channel number of input audio stream
    
    UINT32 drop_cnt;        //!< The frame count droped by avsync module
    UINT32 play_cnt;        //!< The frame count let to play by avsync module
    UINT32 pcm_out_frames;  //!< The frame count have wrote in sound device DMA 
    
    UINT32 pcm_dma_base;    //!< The DMA buffer stored PCM data
    UINT32 pcm_dma_len;     //!< The length of DMA buffer
    UINT32 pcm_rd;          //!< The read index of DMA buffer
    UINT32 pcm_wt;          //!< The write index of DMA buffer
    UINT32 underrun_cnts;   //!< The underrun counts of DMA buffer
    
    UINT32 pcm_dump_addr;   //!< The buffer address dumped pcm data
    UINT32 pcm_dump_len;    //!< The length of dump pcm buffer

    UINT8 spdif_mode;       //!< The output mode of SPDIF interface
    UINT32 spdif_user_bit;  //!< Do not use any more
};



/*!
@}
*/

/*!
@}
*/



#endif


