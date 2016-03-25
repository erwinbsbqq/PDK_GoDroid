#ifndef __ADR_APE_H__
#define __ADR_APE_H__

/*! @addtogroup ape
 *  @{
 */
#ifdef __cplusplus
extern "C" {
#endif

#define AUD_MERGE_BLOCK // add by jacket for merge audio blocks

// decode error type (not supported features)
#define VDEC_ERR_NONE             0
#define VDEC_ERR_SHAPE           (1 << 0)  // shape coding, unsupported
#define VDEC_ERR_SPRITE          (1 << 1)  // static sprite, unsupported
#define VDEC_ERR_NOT8BIT         (1 << 2)  // video data precision N-bit, unsupported
#define VDEC_ERR_NEWPRED         (1 << 3)  // newpred mode, unsupported
#define VDEC_ERR_SCALAB          (1 << 4)  // scalability, unsupported
#define VDEC_ERR_REDUCEDRES      (1 << 5)  // reduced resolution vop, unsupported
#define VDEC_ERR_3POINTGMC       (1 << 6)  // 3-point gmc, video may appear distorted
#define VDEC_ERR_DATA_PARTITION  (1 << 7)  // data partition, unsupported
#define VDEC_ERR_RESOLUTION      (1 << 8)  // resolution unsupported
#define VDEC_ERR_CODEC           (1 << 9)  // codec unsupported
#define VDEC_ERR_NOMEMORY        (1 << 10) // not enough memory

// VdecInit.decoder_flag is a set of bit flags
#define VDEC_FLAG_HAS_LICENSED   (1 << 0)  // Has full license
#define VDEC_FLAG_AVC1_FORMAT    (1 << 1)  // AVC nalu has nalu_size on the head
#define VDEC_FLAG_MPEG4_DECODER  (1 << 2)  // I'm a general mpeg4 decoder

#define APE_VDEC_FLAG_AVC1_FORMAT     (1 << 1)  // AVC nalu has nalu_size on the head
#define APE_MKTAG(a, b, c, d)	      (a | (b << 8) | (c << 16) | (d << 24))
#define ape_h264	                APE_MKTAG('h','2','6','4') //!< H.264 codec
#define ape_xvid                    APE_MKTAG('x','v','i','d') //!< xvid codec
#define ape_mpg2                    APE_MKTAG('m','p','g','2') //!< mpeg2 codec
#define ape_flv1                    APE_MKTAG('f','l','v','1') //!< flv1 codec
#define ape_mjpg			    	APE_MKTAG('m','j','p','g') //!< mjpg codec
#define ape_rmvb                    APE_MKTAG('r','m','v','b') //!< rmvb codec
#define ape_vp8 				   	APE_MKTAG('v','p','8',' ') //!< vp8 codec
#define ape_vc1 					APE_MKTAG('v','c','1',' ')
#define ape_XD 					APE_MKTAG('x','d',' ',' ')
#define ape_wvc1                    APE_MKTAG('w','v','c','1')
#define ape_wmva                    APE_MKTAG('w','m','v','a')
#define ape_wmv3                    APE_MKTAG('w','m','v','3')

typedef enum AD_CodecID {
    APE_CODEC_ID_NONE,

    /* various PCM "codecs" */
    APE_CODEC_ID_PCM_S16LE= 0x10000,
    APE_CODEC_ID_PCM_S16BE,
    APE_CODEC_ID_PCM_U16LE,
    APE_CODEC_ID_PCM_U16BE,
    APE_CODEC_ID_PCM_S8,
    APE_CODEC_ID_PCM_U8,
    APE_CODEC_ID_PCM_MULAW,
    APE_CODEC_ID_PCM_ALAW,
    APE_CODEC_ID_PCM_S32LE,
    APE_CODEC_ID_PCM_S32BE,
    APE_CODEC_ID_PCM_U32LE,
    APE_CODEC_ID_PCM_U32BE,
    APE_CODEC_ID_PCM_S24LE,
    APE_CODEC_ID_PCM_S24BE,
    APE_CODEC_ID_PCM_U24LE,
    APE_CODEC_ID_PCM_U24BE,
    APE_CODEC_ID_PCM_S24DAUD,
    APE_CODEC_ID_PCM_ZORK,
    APE_CODEC_ID_PCM_S16LE_PLANAR,
    APE_CODEC_ID_PCM_DVD,
    APE_CODEC_ID_PCM_F32BE,
    APE_CODEC_ID_PCM_F32LE,
    APE_CODEC_ID_PCM_F64BE,
    APE_CODEC_ID_PCM_F64LE,
    APE_CODEC_ID_PCM_BLURAY,
    APE_CODEC_ID_PCM_LXF,
    APE_CODEC_ID_S302M,

    /* various ADPCM codecs */
    APE_CODEC_ID_ADPCM_IMA_QT= 0x11000,
    APE_CODEC_ID_ADPCM_IMA_WAV,
    APE_CODEC_ID_ADPCM_IMA_DK3,
    APE_CODEC_ID_ADPCM_IMA_DK4,
    APE_CODEC_ID_ADPCM_IMA_WS,
    APE_CODEC_ID_ADPCM_IMA_SMJPEG,
    APE_CODEC_ID_ADPCM_MS,
    APE_CODEC_ID_ADPCM_4XM,
    APE_CODEC_ID_ADPCM_XA,
    APE_CODEC_ID_ADPCM_ADX,
    APE_CODEC_ID_ADPCM_EA,
    APE_CODEC_ID_ADPCM_G726,
    APE_CODEC_ID_ADPCM_CT,
    APE_CODEC_ID_ADPCM_SWF,
    APE_CODEC_ID_ADPCM_YAMAHA,
    APE_CODEC_ID_ADPCM_SBPRO_4,
    APE_CODEC_ID_ADPCM_SBPRO_3,
    APE_CODEC_ID_ADPCM_SBPRO_2,
    APE_CODEC_ID_ADPCM_THP,
    APE_CODEC_ID_ADPCM_IMA_AMV,
    APE_CODEC_ID_ADPCM_EA_R1,
    APE_CODEC_ID_ADPCM_EA_R3,
    APE_CODEC_ID_ADPCM_EA_R2,
    APE_CODEC_ID_ADPCM_IMA_EA_SEAD,
    APE_CODEC_ID_ADPCM_IMA_EA_EACS,
    APE_CODEC_ID_ADPCM_EA_XAS,
    APE_CODEC_ID_ADPCM_EA_MAXIS_XA,
    APE_CODEC_ID_ADPCM_IMA_ISS,
    APE_CODEC_ID_ADPCM_G722,

    /* AMR */
    APE_CODEC_ID_AMR_NB= 0x12000,
    APE_CODEC_ID_AMR_WB,

    /* RealAudio codecs*/
    APE_CODEC_ID_RA_144= 0x13000,
    APE_CODEC_ID_RA_288,

    /* various DPCM codecs */
    APE_CODEC_ID_ROQ_DPCM= 0x14000,
    APE_CODEC_ID_INTERPLAY_DPCM,
    APE_CODEC_ID_XAN_DPCM,
    APE_CODEC_ID_SOL_DPCM,

    /* audio codecs */
    APE_CODEC_ID_MP2= 0x15000,
    APE_CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    APE_CODEC_ID_AAC,
    APE_CODEC_ID_AC3,
    APE_CODEC_ID_DTS,
    APE_CODEC_ID_VORBIS,
    APE_CODEC_ID_DVAUDIO,
    APE_CODEC_ID_WMAV1,
    APE_CODEC_ID_WMAV2,
    APE_CODEC_ID_MACE3,
    APE_CODEC_ID_MACE6,
    APE_CODEC_ID_VMDAUDIO,
    APE_CODEC_ID_SONIC,
    APE_CODEC_ID_SONIC_LS,
    APE_CODEC_ID_FLAC,
    APE_CODEC_ID_MP3ADU,
    APE_CODEC_ID_MP3ON4,
    APE_CODEC_ID_SHORTEN,
    APE_CODEC_ID_ALAC,
    APE_CODEC_ID_WESTWOOD_SND1,
    APE_CODEC_ID_GSM, ///< as in Berlin toast format
    APE_CODEC_ID_QDM2,
    APE_CODEC_ID_COOK,
    APE_CODEC_ID_TRUESPEECH,
    APE_CODEC_ID_TTA,
    APE_CODEC_ID_SMACKAUDIO,
    APE_CODEC_ID_QCELP,
    APE_CODEC_ID_WAVPACK,
    APE_CODEC_ID_DSICINAUDIO,
    APE_CODEC_ID_IMC,
    APE_CODEC_ID_MUSEPACK7,
    APE_CODEC_ID_MLP,
    APE_CODEC_ID_GSM_MS, /* as found in WAV */
    APE_CODEC_ID_ATRAC3,
    APE_CODEC_ID_VOXWARE,
    APE_CODEC_ID_APE,
    APE_CODEC_ID_NELLYMOSER,
    APE_CODEC_ID_MUSEPACK8,
    APE_CODEC_ID_SPEEX,
    APE_CODEC_ID_WMAVOICE,
    APE_CODEC_ID_WMAPRO,
    APE_CODEC_ID_WMALOSSLESS,
    APE_CODEC_ID_ATRAC3P,
    APE_CODEC_ID_EAC3,
    APE_CODEC_ID_SIPR,
    APE_CODEC_ID_MP1,
    APE_CODEC_ID_TWINVQ,
    APE_CODEC_ID_TRUEHD,
    APE_CODEC_ID_MP4ALS,
    APE_CODEC_ID_ATRAC1,
    APE_CODEC_ID_BINKAUDIO_RDFT,
    APE_CODEC_ID_BINKAUDIO_DCT,
    APE_CODEC_ID_AAC_LATM,
    APE_CODEC_ID_QDMC,
    APE_CODEC_ID_CELT,

    /* subtitle codecs */
    APE_CODEC_ID_DVD_SUBTITLE= 0x17000,
    APE_CODEC_ID_DVB_SUBTITLE,
    APE_CODEC_ID_TEXT,  ///< raw UTF-8 text
    APE_CODEC_ID_XSUB,
    APE_CODEC_ID_SSA,
    APE_CODEC_ID_MOV_TEXT,
    APE_CODEC_ID_HDMV_PGS_SUBTITLE,
    APE_CODEC_ID_DVB_TELETEXT,
    APE_CODEC_ID_SRT,
    APE_CODEC_ID_MICRODVD,

    /* other specific kind of codecs (generally used for attachments) */
    APE_CODEC_ID_TTF= 0x18000,

    APE_CODEC_ID_PROBE= 0x19000, ///< codec_id is not known (like APE_CODEC_ID_NONE) but lavf should attempt to identify it

    APE_CODEC_ID_MPEG2TS= 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
                                * stream (only used by libavformat) */
    APE_CODEC_ID_FFMETADATA=0x21000,   ///< Dummy codec for streams containing only metadata information.
}audio_codec_id_t;

struct ape_av_packet 
{
    /**
     * Presentation timestamp in AVStream->time_base units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in AVPacket.
     */
    long long pts;
    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    long long dts;
    unsigned char *data;
    int size;
    int stream_index;
    int flags;
    unsigned short width, height;
    unsigned int param_change_flags;

    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    int duration;
    void  (*destruct)(struct ape_av_packet *);
    void  *priv;
    long long pos;                            ///< byte position in stream, -1 if unknown
    long long convergence_duration;
};

enum ape_av_picture_type 
{
    APE_PICTURE_TYPE_I = 1, ///< Intra
    APE_PICTURE_TYPE_P,     ///< Predicted
    APE_PICTURE_TYPE_B,     ///< Bi-dir predicted
    APE_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG4
    APE_PICTURE_TYPE_SI,    ///< Switching Intra
    APE_PICTURE_TYPE_SP,    ///< Switching Predicted
    APE_PICTURE_TYPE_BI,    ///< BI type
};

struct ape_av_panscan{
    /**
     * id
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    int id;

    /**
     * width and height in 1/16 pel
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    int width;
    int height;

    /**
     * position of the top left corner in 1/16 pel for up to 3 fields/frames
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    short position[3][2];
};

/**
 * rational number numerator/denominator
 */
struct ape_av_rational
{
    int num; ///< numerator
    int den; ///< denominator
};

struct ape_av_frame 
{
     /**
     * pointer to the picture planes.
     * This might be different from the first allocated byte
     * - encoding: 
     * - decoding: 
     */
    unsigned char *data[4];
    int linesize[4];
    /**
     * pointer to the first allocated byte of the picture. Can be used in get_buffer/release_buffer.
     * This isn't used by libavcodec unless the default get/release_buffer() is used.
     * - encoding: 
     * - decoding: 
     */
    unsigned char *base[4];
    /**
     * 1 -> keyframe, 0-> not
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    int key_frame;
    /**
     * Picture type of the frame, see ?_TYPE below.
     * - encoding: Set by libavcodec. for coded_picture (and set by user for input).
     * - decoding: Set by libavcodec.
     */
    enum ape_av_picture_type pict_type;
    /**
     * presentation timestamp in time_base units (time when frame should be shown to user)
     * If AV_NOPTS_VALUE then frame_rate = 1/time_base will be assumed.
     * - encoding: MUST be set by user.
     * - decoding: Set by libavcodec.
     */
    long long  pts;
    /**
     * picture number in bitstream order
     * - encoding: set by
     * - decoding: Set by libavcodec.
     */
    int coded_picture_number;
    /**
     * picture number in display order
     * - encoding: set by
     * - decoding: Set by libavcodec.
     */
    int display_picture_number;
    /**\
     * quality (between 1 (good) and FF_LAMBDA_MAX (bad)) 
     * - encoding: Set by libavcodec. for coded_picture (and set by user for input).
     * - decoding: Set by libavcodec.
     */
    int quality; 
    /**
     * buffer age (1->was last buffer and dint change, 2->..., ...).
     * Set to INT_MAX if the buffer has not been used yet.
     * - encoding: unused
     * - decoding: MUST be set by get_buffer().
     */
    int age;
    /**
     * is this picture used as reference
     * The values for this are the same as the MpegEncContext.picture_structure
     * variable, that is 1->top field, 2->bottom field, 3->frame/both fields.
     * Set to 4 for delayed, non-reference frames.
     * - encoding: unused
     * - decoding: Set by libavcodec. (before get_buffer() call)).
     */
    int reference;
    /**
     * QP table
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    char *qscale_table;
    /**
     * QP store stride
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    int qstride;
    /**
     * mbskip_table[mb]>=1 if MB didn't change
     * stride= mb_width = (width+15)>>4
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    unsigned char *mbskip_table;
    /**
     * motion vector table
     * @code
     * example:
     * int mv_sample_log2= 4 - motion_subsample_log2;
     * int mb_width= (width+15)>>4;
     * int mv_stride= (mb_width << mv_sample_log2) + 1;
     * motion_val[direction][x + y*mv_stride][0->mv_x, 1->mv_y];
     * @endcode
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    unsigned short (*motion_val[2])[2];
    /**
     * macroblock type table
     * mb_type_base + mb_width + 2
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    unsigned int *mb_type;
    /**
     * log2 of the size of the block which a single vector in motion_val represents: 
     * (4->16x16, 3->8x8, 2-> 4x4, 1-> 2x2)
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    unsigned char motion_subsample_log2;
    /**
     * for some private data of the user
     * - encoding: unused
     * - decoding: Set by user.
     */
    void *opaque;
    /**
     * error
     * - encoding: Set by libavcodec. if flags&CODEC_FLAG_PSNR.
     * - decoding: unused
     */
    long long  error[4];
    /**
     * type of the buffer (to keep track of who has to deallocate data[*])
     * - encoding: Set by the one who allocates it.
     * - decoding: Set by the one who allocates it.
     * Note: User allocated (direct rendering) & internal buffers cannot coexist currently.
     */
    int type;
    /**
     * When decoding, this signals how much the picture must be delayed.
     * extra_delay = repeat_pict / (2*fps)
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    int repeat_pict;
    /**
     * 
     */
    int qscale_type;
    /**
     * The content of the picture is interlaced.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec. (default 0)
     */
    int interlaced_frame;
    /**
     * If the content is interlaced, is top field displayed first.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    int top_field_first;
    /**
     * Pan scan.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    struct ape_av_panscan *pan_scan;
    /**
     * Tell user application that palette has changed from previous frame.
     * - encoding: ??? (no palette-enabled encoder yet)
     * - decoding: Set by libavcodec. (default 0).
     */
    int palette_has_changed;
    /**
     * codec suggestion on buffer type if != 0
     * - encoding: unused
     * - decoding: Set by libavcodec. (before get_buffer() call)).
     */
    int buffer_hints;
    /**
     * DCT coefficients
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    short *dct_coeff;
    /**
     * motion reference frame index
     * the order in which these are stored can depend on the codec.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    char *ref_index[2];
    /**
     * reordered opaque 64bit (generally an integer or a double precision float
     * PTS but can be anything). 
     * The user sets AVCodecContext.reordered_opaque to represent the input at
     * that time,
     * the decoder reorders values as needed and sets AVFrame.reordered_opaque
     * to exactly one of the values provided by the user through AVCodecContext.reordered_opaque
     * @deprecated in favor of pkt_pts
     * - encoding: unused
     * - decoding: Read by user.
     */
    long long   reordered_opaque;
    /**
     * hardware accelerator private data (FFmpeg allocated)
     * - encoding: unused
     * - decoding: Set by libavcodec
     */
    void *hwaccel_picture_private;
    /**
     * reordered pts from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    long long pkt_pts;
    /**
     * dts from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    long long  pkt_dts;
    /**
     * the AVCodecContext which ff_thread_get_buffer() was last called on
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    //struct AVCodecContext *owner;
    void *owner;
    /**
     * used by multithreading to store frame-specific info
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    void *thread_opaque;
    /**
     * frame timestamp estimated using various heuristics, in stream time base
     * - encoding: unused
     * - decoding: set by libavcodec, read by user.
     */
    long long best_effort_timestamp;
    /**
     * reordered pos from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    long long  pkt_pos;
    /**
     * reordered sample aspect ratio for the video frame, 0/1 if unknown\unspecified
     * - encoding: unused
     * - decoding: Read by user.
     */
    struct ape_av_rational sample_aspect_ratio;
    /**
     * width and height of the video frame
     * - encoding: unused
     * - decoding: Read by user.
     */
    int width, height;
    /**
     * format of the frame, -1 if unknown or unset
     * It should be cast to the corresponding enum (enum PixelFormat
     * for video, enum AVSampleFormat for audio)
     * - encoding: unused
     * - decoding: Read by user.
     */
    int format;
};

enum ape_pts_type
{
	APS_PTS_Frame,  //!PTS of each frame
	APE_PTS_Display,//!PTS by the display sequence
};

/*! @enum BUFFER_FORMAT
@brief ��һ��ö�ٶ����˲�ͬ��;��buffer��index��ͨ����Щindex���Ի����Ӧbuffer���豸ID
*/
enum BUFFER_FORMAT
{
	VPKT_HDR,
	VPKT_DATA,
	DECV_OUT,
	DISP_IN,
	APKT_HDR,
	APKT_DATA,
	DECA_OUT,
	SND_IN,
	CLOCK0,
	CLOCK1
};

/*!@struct video_config_info_t
@brief ����ṹ���������ó�ʼ��video decoder����Ҫ����ƵͼƬ�Ļ�����Ϣ*/

typedef struct Video_Config_Info_T
{
	int width;//!<ͼƬ����
	int height;//!<ͼƬ���
	int time_base_num;//!<ʱ��������Ϣ
	int time_base_den;//!<ʱ��������Ϣ
	int sample_aspect_ratio_num;//!<��Ƶ��߱������Ϣ
	int sample_aspect_ratio_den;//!<��Ƶ��߱������Ϣ
	int preview;//!<�Ƿ���preview
	int decoder_flag;//!<decoder����Ҫ��һЩ��־�������Ƿ�Ϊrv8
	unsigned int fourcc;//!<����mpeg4

	int timescale_num;//!ʱ�������
	int timescale_den;//! ʱ�����ĸ	

	enum ape_pts_type pts_type;
} video_config_info_t;
/*!@struct avsync_config_info_t
@brief ����ṹ����������avsync��ز���*/
typedef struct Avsync_Config_Info_T
{
	int av_sync_mode;//!<�Ƿ���ͬ��1����ͬ����0������ͬ��
	int av_sync_unit;//!<��ʱδʹ�ã���ֵΪ0����
	int hold_threshold;
	int drop_threshold;
} avsync_config_info_t;
/*!@struct audio_config_info_t
@brief ����ṹ���������ó�ʼ��audio decoder����Ҫ�Ļ�����Ϣ*/
typedef struct Audio_Config_Info_T
{
	int av_sync_mode;//!<�Ƿ���ͬ�� 0������ͬ����1����audioΪ��׼��2����video����׼
	int av_sync_unit;//!<��ʱδʹ�ã���ֵΪ0����
	int codec_id;//!<��Ӧ��codec_id����Ӧ��ö������CodecID��ͷ�ļ�avformat.h�ж���
	int sample_rate;//!<������
	int bits_per_coded_sample;
	int channels;//!<����
	int bit_rate;//!<������
	int block_align;
    int codec_frame_size;
	unsigned char* extradata;
    unsigned int  extradata_size;

	int timescale_num;//!ʱ�������
	int timescale_den;//! ʱ�����ĸ	
} audio_config_info_t;
/*!@struct audio_config_info_t
@brief ����ṹ���������ó�ʼ��audio merge����Ҫ�Ļ�����Ϣ*/
#define MERGE_BLOCK_COUNT 2
#define MERGE_DATA_LENGTH 1024*16
typedef struct Audio_Merge_Info_T
{
    unsigned int merge_block_cnt;
    
    unsigned int hdr_merge_time;
    unsigned int hdr_merge_data_size;
    unsigned int *last_hdr_buf;
    unsigned int last_hdr_len;
    struct av_packet last_pkt_info;

    unsigned int data_merge_size;
    unsigned int data_merge_time;
    unsigned char *data_merge_buf;
    unsigned char *data_merge_buf_tmp;
    unsigned int data_merge_len;
} audio_merge_info_t; // add by jacket for merge audio block
/*!@struct reset_info_t
@brief ��seek����reset�ӿ�ʱʹ������ṹ����д��Σ���ʱ�ò���*/
typedef struct Reset_info_T
{
	int qmax;
	int qmin;
} reset_info_t;
/*!@struct video_status_t
@brief ����ṹ��������decoder��ȡ�ѽ���֡�Ĳ�����Ϣ*/
typedef struct Video_Status_T
{
	unsigned int decode_status;//!<����״̬������first_header_got
    unsigned int pic_width;    //!<�ѽ���ͼƬ�ĳ���
    unsigned int pic_height;   //!<�ѽ���ͼƬ�Ŀ��
    unsigned int frame_rate;   //!<֡��
    unsigned int decoder_feature; //!<using for mpeg4 or not
    unsigned int decode_error;
    unsigned int sar_width;
    unsigned int sar_height;
    unsigned int interlaced_frame;
    unsigned int top_field_first;
    unsigned int frames_decoded;
    unsigned int frames_displayed;
	unsigned int buffer_size;
    unsigned int buffer_used;
	
    long long frame_last_pts;
	int  first_pic_showed;    
   
    int first_header_got;      //!<�Ƿ��Ѿ����˵�һ֡
	int video_is_support;	   //!<��video��ʽ�Ƿ�֧��
} video_status_t;

/*!
@brief ��APEģ�飬����ģ���ڲ���Դ
@return int
@retval  0        �򿪳ɹ���
@retval  -1       ��ʧ�ܡ�
*/
int ape_open(void);
/*!
@brief �ص�APEģ��
@return int
@retval  0        �رճɹ���
@retval  -1       �ر�ʧ�ܡ�
*/
int ape_close(void);
void ape_av_output_mode(int flag);
/*!
@brief seekʱ����audio decoder
*/
void ape_audio_reset();
/*!
@brief audio ������ͣ
@param[in] pause_decode �Ƿ���ͣ��1����ͣ��0������ͣ
@param[in] pause_output �Ƿ�ص�sound����� 1���ص���0�����ص�
@return int
@retval  0       ��ͣ�ɹ�
@retval  -1      ��ͣʧ��
*/
int ape_audio_pause(int pause_decode,int pause_output);
/*!
@brief ���buffer��pkt data
@return int
@retval  0        �����ɹ�
@retval  -1       ����ʧ��
*/
int ape_audio_buffer_clear();
/*!
@brief ֹͣ����audio
@return int
@retval  0        �����ɹ�
@retval  -1       ����ʧ��
*/
int ape_audio_release();
/*!
@brief audio decoder�ĳ�ʼ��
@param[in] config_info audio decoder�ĳ�ʼ����Ϣ
@param[in] reset decoder����Ҫ���͵Ĳ�������������Ҫ��0����
@return int
@retval  0       ��ʼ���ɹ�
@retval  -1      ��ʼ��ʧ��
*/
int ape_adec_init(audio_config_info_t* config_info, int reset);

int ape_vdec_init(unsigned int codec_tag, video_config_info_t* info);

/*!
@brief ���vbv buffer��pkt data
@return int
@retval  0        �����ɹ�
@retval  -1       ����ʧ��
*/
int ape_video_buffer_clear();
/*!
@brief ֹͣ����video
@param[in] codec_tag ���������ͣ�xvid,h264,mpg2,rmvb,vp8,vc1
@return int
@retval  0      �رճɹ�
@retval  -1     �ر�ʧ��
*/
int ape_vdec_release(unsigned int codec_tag);
/*!
@brief ��Ӧcodec_tag��video decoder�ĳ�ʼ��
@param[in] codec_tag ���������ͣ�xvid,h264,mpg2,rmvb,vp8,vc1
@param[in] info video decoder��ʼ������
@return int
@retval  0      ��ʼ���ɹ�
@retval  -1     ��ʼ��ʧ��
*/
int ape_vdec_reset(unsigned int codec_tag, reset_info_t* reset_info);
/*!
@brief video ������ͣ
@param[in] pause_decode �Ƿ���ͣ��1����ͣ��0������ͣ
@param[in] pause_output �Ƿ�ص�display����� 1���ص���0�����ص�
@return int
@retval  0       ��ͣ�ɹ�
@retval  -1      ��ͣʧ��
*/
int ape_vdec_pause(int pause_decode,int pause_output);
/*!
@brief ��Ƶ����Ԥ������
@param[in] x ���Ŵ��ڵ����Ͻ�x����
@param[in] y ���Ŵ��ڵ����Ͻ�y����
@param[in] width  ���Ŵ��ڵĳ���
@param[in] height ���Ŵ��ڵĸ߶�
@return int
@retval  0       ���óɹ�
@retval  -1      ����ʧ��
*/
int ape_video_zoom(int x, int y, int width, int  height);

/*!
@brief ��Ƶ������ת����
@param[in] frame_angle ������ת�Ƕ�
@return int
@retval  0       ���óɹ�
@retval  -1      ����ʧ��
*/
int ape_video_rotate(enum VDecRotationAngle frame_angle);

/*!
@brief ����avsync����
@param[in] info avͬ����������Ӧ�ṹ��avsync_config_info_t
@return int
@retval  0       ���óɹ�
@retval  -1      ����ʧ��
*/
int ape_vdec_avsync_config(avsync_config_info_t* info);
/*!
@brief seekʱ����video decoder
@param[in] codec_tag ���������ͣ�xvid,h264,mpg2,rmvb,vp8,vc1
@param[in] reset_info ����video decoder�Ĳ�������Ӧ�ṹ��reset_info_t
@return int
@retval  0      ���óɹ�
@retval  -1     ����ʧ��
*/
int ape_vdec_reset(unsigned int codec_tag,reset_info_t* reset_info);
/*!
@brief ����extra_data
@param[in] extradata extradata��ŵĵ�ַ
@param[in] extradata_size extradata�Ĵ�С
@return int
@retval  0      ����ɹ�
@retval  -1     ����ʧ��
*/
int ape_vdec_extra_data(unsigned char * extradata, unsigned int * extradata_size);

/*!
@brief ��decoder����ѽ���֡����Ϣ
@param[out] status ������Ϣ�ĵ�ַ
@return int
@retval  0      ���״̬�ɹ�
@retval  -1     ���״̬ʧ��
*/
int ape_vdec_get_status(video_status_t *video_status);

/*!
@brief �ͷ�����Ƶ����������Ļ���buffer
*/
void ape_free_av_buf();
/*!
@brief �������Ҫ����buffer��fd
@param[in] buf_format buffer���ͣ���Ӧö������BUFFER_FORMAT
@return int
@retval  �豸fd
*/
int ape_get_buf_fd(enum BUFFER_FORMAT buf_format);
/*!
@brief ��������Ƶ��������Ҫ�Ļ���buffer
@return int
@retval  0      malloc buffer�ɹ�
@retval  -1     malloc bufferʧ��
*/
int ape_malloc_av_buf();
/*!
@brief ��ö�Ӧfd��������ܵ�buffer size
@param[in] fd ��Ӧbuffer���豸fd
@return int
@retval  Total buffer size
*/
int ape_show_total_buf_size(int fd);
/*!
@brief ��ö�Ӧfd���е�buffer size
@param[in] fd ��Ӧbuffer���豸fd
@return int
@retval  free buffer size
*/
int ape_show_free_buf_size(int fd);
/*!
@brief ��ö�Ӧfd���ж�������û��ʹ�ã������ж��ٴ����������
@param[in] fd ��Ӧbuffer���豸fd
@return int
@retval  valid buffer size
*/
int ape_show_valid_buf_size(int fd);
/*!
@brief �Ӷ�Ӧ��fd�л�ȡһ����С����Ϣ
@param[in] fd ��Ӧbuffer���豸fd
@param[out] addr ��ϢҪ��ŵĵ�ַ
@param[out] size Ҫ�����ݵĴ�С
@return int
@retval  0       ��ȡ�ɹ�
@retval  -1      ��ȡʧ��
*/
int ape_read_info(int fd, void* addr, int size );
/*!
@brief ��һ����������д�뵽fd��Ӧ��buffer��
@param[in] fd ��Ӧbuffer���豸fd
@param[in] addr ��ǰ���ݴ�ŵĵ�ַ
@param[in] size Ҫд���ݵĴ�С
@return int
@retval  0       д���ݳɹ�
@retval  -1      д����ʧ��
*/
int ape_write_data(int fd, void* addr, int size );
/*!
@brief ���ö�Ӧclock_fd��divisor
@param[in] clock_fd ��ѡ���clock�豸fd
@param[in] divisor  ��Ҫ�趨��divisorֵ
@return int
@retval  0      ���óɹ�
@retval  -1     ����ʧ��
*/
int ape_set_clock_divisor(int clock_fd, unsigned int divisor);
/*!
@brief ���ö�Ӧclock_fd��clock�豸
@param[in] clock_fd ��ѡ���clock�豸fd
@param[in] valid �Ƿ����ã�0:�����ã�1������
@return int
@retval  0      ���óɹ�
@retval  -1     ����ʧ��
*/
int ape_set_clock_valid(int clock_fd, unsigned int valid);
/*!
@brief ��ͣ��Ӧclock_fd��ʹ��
@param[in] clock_fd ��ѡ���clock�豸fd
@param[in] flag �Ƿ���ͣ��1����ͣʹ�ã�0������ʹ��
@return int
@retval  0      ���óɹ�
@retval  -1     ����ʧ��
*/
int ape_clock_pause(int clock_fd, unsigned int flag);
/*!
@brief ���ö�Ӧclock_fd��ֵ
@param[in] clock_fd ��ѡ���clock�豸fd
@param[in] value ��Ҫ�趨clock��ֵ
@return int
@retval  0      ���óɹ�
@retval  -1     ����ʧ��
*/
int ape_write_clock(int clock_fd, unsigned int value);
/*!
@brief ��ȡ��Ӧclock_fd��ֵ
@param[in] clock_fd ��Ӧbuffer���豸fd
@return int
@retval  ��Ӧ��clockֵ
*/
unsigned int ape_read_clock(int clock_fd);
/*!
@brief ��ö�Ӧformat��clock���豸fd
@param[in] buf_format ��ѡ���clock���ͣ���Ӧö������BUFFER_FORMAT�е�CLOCK0,CLOCK1
@return int
@retval  ��Ӧ��clock�豸id
*/
int ape_get_clock_fd(enum BUFFER_FORMAT buf_format);

/*!
@brief ���ô򿪻��߹ر�dump���ݵ�disk�Ĺ���
@param[in] file_name ������ļ�����
@param[in] flag dump���ݵĿ��أ�1:��ʼdump���ݣ�0:����dump����
@return int
@retval  0      ���óɹ�
@retval  -1     ����ʧ��
*/
int ape_dump_data_to_disk(const char* file_name, unsigned int flag);

/*!
@brief ��ȡboot media���� ��ʶλ
@return int
@retval  0x88      �Ѿ�����
@retval  other    û�н���
*/
unsigned char ape_get_boot_media_flag(void);

/*!
@brief ���ÿ�����˱�־
@return int
@retval  0          ���óɹ�
@retval  -1        ����ʧ�� 
*/
int ape_vdec_trickseek(unsigned int status);

/*!

@brief ���ÿ�����˱�־
@return int
@retval  0          ���óɹ�
@retval  -1        ����ʧ�� 
*/
int ape_vdec_trickseek(unsigned int status);



/*!
@brief ע��callback����
@return int
@retval  0          ע��ɹ�
@retval  -1        ע��ʧ�� 
*/
int ape_vdec_reg_cb(struct vdec_io_reg_callback_para  *pPara);

int ape_vdec_get_trick_status();

void ape_video_rotation_enable(unsigned char flag, unsigned int angle);

/*!

@brief  ape ʹ��BDMA  �����user space to kernel space ���ݿ���
@param[in] enable  1:enable; 0:disable
@return int
@retval  0          ���óɹ�
@retval  -1        ����ʧ�� 
*/
int ape_set_bdma_mode(int enable);

/*!
@brief �������ŵĽ�Ŀ
@param[in] file_path�������ļ���
*/
void ape_dump_enable(void *file_path);

/*!
@brief ��ʼape�ط�
@param[in] file_path�طŵ��ļ���
*/
void ape_playback_start(void *file_path);

/*!
@brief ֹͣape�ط�
*/
void ape_playback_stop(void);

/*!
@brief ��ȡape�Ĳ���״̬
@return int
@retval  0 -- �Ѿ�ֹͣ
@retval  1 -- ���ڲ���
*/
int ape_playback_status(void);

/*!
@brief ������ƵRGB��ʽ���
@return int
@retval  0 -- ���óɹ�
@retval  -1 -- ����ʧ��
*/
int ape_enable_rgb_output(void);
/*!
@brief ����audio mergeģʽ
@return int
@retval  0 -- ������audio block merge
@retval  1 -- ����audio block merge
*/
int ape_set_audio_merge_mode(unsigned int mode);
/*!
@brief ����audio merge����
@return int
*/
int ape_set_merge_count(unsigned int merge_block_cnt);

/*!
@brief ֹͣ����MKV��Ƶ�Ĳ���
@return int
*/
void ape_stop_bootmedia();


#ifdef __cplusplus 
} 

#endif
/*!
@}
*/
#endif
