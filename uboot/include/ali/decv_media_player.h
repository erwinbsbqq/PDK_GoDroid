#ifndef _DECV_MEDIA_PLAYER_H_
#define _DECV_MEDIA_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

// decode status machine
#define VDEC_NEW_VIDEO_PACKET     1     // ready to decode new video packet(picture)
#define VDEC_WAIT_NEW_FB          2     // waiting for free buffer to decode video
#define VDEC_NEW_SLICE            3     // ready to decode new video slice
#define VDEC_ON_GOING             4     // video decoding is on going
#define VDEC_POST_PROCESS         5     // last picture decoding is done, ready to do some post process
#define VDEC_CONTINUE             6     // decore needs to push last decoded P VOP into DQ in B-VOP decoding and continue B-VOP decoding

// decode mode
#define VDEC_MODE_NORMAL          0     // normal decode
#define VDEC_MODE_VOL             1     // parse all headers above (including) VOP level without VOP reconstuction
#define VDEC_MODE_HEADER          2     // parse header to get current frame's prediction type
#define VDEC_MODE_SKIP_B_FRAME    3     // skip b frame
#define VDEC_MODE_SKIP_B_P_FRAME  4     // only decode i frame
#define VDEC_MODE_SBM             5     //

// decoder command
#define VDEC_CMD_INIT             0     // Initialize the decoder
#define VDEC_CMD_RELEASE          1     // Release the decoder
#define VDEC_CMD_DECODE_FRAME     2     // Decode a frame
#define VDEC_CMD_SW_RESET         3     // Reset the decoder sw status.
#define VDEC_CMD_HW_RESET         4     // Reset the HW to be ready to decode next frame
#define VDEC_CMD_EXTRA_DADA       5     // Decode extra data before decode frame
#define VDEC_CMD_GET_STATUS       6     // Get decoder status
#define VDEC_CMD_PAUSE_DECODE     7     // Pause decode task running

// display queue operations (void* pHandle, int decOpt, void* pParam1, void* pParam2)
// *pParam1 --(UINT32)de_index if not specified, *pParam2 -- (UINT32)frame_id if available.
// frame_id == (frame_array_idx << 8) | frame_idx. For single output, frame_array_idx == 0
#define VDEC_DQ_INIT              20    // *pParam1 -- de_index, *pParam2 --, int DQ for de_index
#define VDEC_DQ_ADD_NULL_FRAME    21    // *pParam1 -- de_index, *pParam2 --, Add an null frame into DQ
#define VDEC_DQ_PEEK_FRAME        22    // *pParam1 -- de_index, *pParam2 -- the next frame_id in DQ
#define VDEC_DQ_POP_FRAME         23    // *pParam1 -- de_index, *pParam2 -- the frame_id to be poped
#define VDEC_DQ_GET_COUNT         24    // *pParam1 -- de_index, *pParam2 -- the frame number in DQ
#define VDEC_FRAME_PEEK_FREE      25    // *pParam1 -- the next free frm_idx, *pParam2 -- the next free pip_idx
#define VDEC_FRAME_GET_INFO       26    // *pParam2 -- frame_id, pParam1 -- struct DisplayInfo *,
#define VDEC_FRAME_GET_POC        27    // *pParam2 -- frame_id, pParam1 -- int *p_poc,
#define VDEC_FRAME_SET_FREE       28    // *pParam2 -- frame_id, bit16 must be set or clear, see DEC_LAST_REQ_FRAME_FLAG.
#define VDEC_FRAME_REQUESTING     29    // *pParam2 -- frame_id, pParam1 -- struct OutputFrmManager *,
#define VDEC_FRAME_REQUEST_OK     30    // *pParam2 -- frame_id, *pParam1 -- de_index,
#define VDEC_FRAME_RELEASE_OK     31    // *pParam2 -- frame_id, *pParam1 -- de_index,
#define VDEC_CLEAR_SCREEN         32    // *pParam2 -- flag,     *pParam1 -- de_index,
#define VDEC_START_OUTPUT         33    // *pParam2 -- flag,     *pParam1 -- de_index,
#define VDEC_FRAME_GET_PTS        34    // *pParam2 -- frame_id, pParam1 -- UINT32 *pts
#define VDEC_CFG_VIDEO_SBM_BUF    35    // *pParam1 -- smb idx for video pkt buf, *pParam2 -- sbm idx for video output queue
#define VDEC_CFG_DISPLAY_SBM_BUF  36    // *pParam1 -- smb idx for display queue
#define VDEC_CFG_SYNC_MODE        37    // *pParam1 -- av sync mode
#define VDEC_CFG_SYNC_THRESHOLD   38    // *pParam1 -- hold threshold, *pParam2 -- drop threshold
#define VDEC_CFG_SYNC_DELAY       39    // *pParam1 -- av sync delay [-500ms, 500ms]
#define VDEC_CFG_DISPLAY_RECT     40    // *pParam1 -- source rect, *pParam2 destination rect
#define VDEC_CFG_DECODE_MODE      41    // *pParam1 -- decode mode
#define VDEC_CFG_FREERUN_TRD      42    // *pParam1 -- hol2free threshold, *pParam2 -- drop2free threshold

// decode return values
#define VDEC_SUCCESS              0
#define VDEC_FAILURE              1
#define VDEC_FORBIDDEN_DECODE     2     // some reason cannot decode
#define VDEC_INVALID_CMD          3
#define VDEC_FRAME_SKIPED         4     // frame skiped according to decode mode
#define VDEC_NO_MEMORY            5     // memory not enough
#define VDEC_BAD_FORMAT           6
#define VDEC_NOT_IMPLEMENTED      7     // new feature not supported
#define VDEC_PATCH_PSUDO_MB       8     // Patch IC issue
#define VDEC_CURRENT_CONTINUE     9     // continue decode current frame
#define VDEC_SEND_NEXT            10

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
// other flag
#define VDEC_FLAG_LAST_REQ_FRAME (1 << 16) // An flag to free the last requested frame

#define VDEC_FRM_IDX_NOT_DISPLAY  0xfe     // An indicator for not display frame
#define VDEC_FRM_IDX_INVALID      0xff

#define VDEC_MIN_FRAME_BUF_NUM    4
#define VDEC_MAX_FRAME_BUF_NUM    6

#define MKTAG(a, b, c, d)	      (a | (b << 8) | (c << 16) | (d << 24))
#define h264	                  MKTAG('h','2','6','4')
#define xvid                      MKTAG('x','v','i','d')
#define mpg2                      MKTAG('m','p','g','2')
#define flv1                      MKTAG('f','l','v','1')
#define vp8                       MKTAG('v','p','8',' ')
#define wvc1                      MKTAG('w','v','c','1')
#define rv	  			          MKTAG('r','m','v','b')
typedef UINT32 FourCC;

typedef struct _VdecHWMemConfig
{
    UINT32 fb_y[VDEC_MAX_FRAME_BUF_NUM];
    UINT32 fb_c[VDEC_MAX_FRAME_BUF_NUM];
    UINT32 dv_y[VDEC_MAX_FRAME_BUF_NUM];
    UINT32 dv_c[VDEC_MAX_FRAME_BUF_NUM];
    UINT32 fb_max_stride;
    UINT32 fb_max_height;
    UINT32 dv_max_stride;
    UINT32 dv_max_height;
    UINT32 fb_num;
    UINT32 neighbor_mem;
    UINT32 colocate_mem;
    UINT32 cmdq_base;
    UINT32 cmdq_len;
    UINT32 vbv_start;
    UINT32 vbv_len;
} VdecHWMemConfig;

typedef struct _DViewCtrl
{
    UINT8 dv_enable;
    UINT8 dv_h_scale_factor;
    UINT8 dv_v_scale_factor;
    UINT8 interlaced:1;
    UINT8 bottom_field_first:1;
}DViewCtrl;

// Structure for HW memory config
typedef struct _VdecFBConfig
{
    UINT32 FBStride;
    UINT32 FBMaxHeight;
    UINT8  *FrmBuf;     //used only for SW_DECODE
    UINT8  *DispBuf;    //In HW_DISPLAY mode, first 4 bytes store frame buffer addr, last 4 byte stor dv buffer addr
} VdecFBConfig;

/// Describes a compressed or uncompressed video format.
typedef struct _VdecFormatInfo
{
    FourCC fourcc;
    INT32 bits_per_pixel;
    INT32 pic_width;
    INT32 pic_height;
    INT32 pic_inverted;
    INT32 pixel_aspect_x;
    INT32 pixel_aspect_y;
    INT32 frame_rate;
    INT32 frame_period;
    BOOL frame_period_const;
} VdecFormatInfo;

// decore notify event to video player engine
enum VdecEventType
{
    VDEC_EVENT_FRAME_FINISHED,   // one frame finished by VE
    VDEC_EVENT_FRAME_OUTPUT,     // one frame put into DQ
    VDEC_EVENT_DE_RELEASED,      // one frame released by DE
    VDEC_EVENT_DE_REQUESTING,    // Requesting frame by DE
    VDEC_EVENT_SLICE_FINISHED,   // one slice finished by VE
    VDEC_EVENT_FB_MALLOC_DONE,   // frame buffer malloc done
    VDEC_EVENT_FIELD_PIC_FLAG,   // 0 -- 1 frame 1 pts, 1 -- 1 field 1 pts
};

typedef int (*pfn_on_decode_evnet)(enum VdecEventType event, UINT32 param);
typedef int (VdecDecodeFunction)(void *pHandle, int VdecCmd, void *pParam1, void *pParam2);

// Structure with parameters used to instantiate a decoder.
typedef struct _VdecInit
{
    VdecFormatInfo fmt_out; ///< Desired output video format.
    VdecFormatInfo fmt_in; ///< Given input video format
    const VdecFBConfig *pfrm_buf;
    const VdecHWMemConfig *phw_mem_cfg;
    pfn_on_decode_evnet  on_decode_event;
    T_Request pfn_decore_de_request;
    T_Release pfn_decore_de_release;
    UINT32 decoder_flag;
    UINT32 decode_mode;
    UINT32 preview;
}VdecInit;

// Structure containing compressed video data.
typedef struct _VdecVideoPacket
{
    void *buffer;
    UINT32 size;
    UINT8 decoded;
}VdecVideoPacket;

typedef struct _VdecFrmBuf
{
    UINT8 *buf_addr; //In HW_DISPLAY mode, first 4 bytes store frame buffer addr, last 4 byte stor dv buffer addr
    UINT8 buf_id;
    UINT8 prediction_type;
    DViewCtrl dv_ctrl;
}VdecFrmBuf;

// Structure containing input bitstream and decoder frame buffer.
typedef struct _VdecFrameConfig
{
    INT32 decode_buf_id;
    INT32 decode_finish; ///< Non-zero value means that a frame was successfully decoded.  Set by decoder.
    INT32 decode_mode;
    INT32 decode_status;
    UINT32 pts;
    VdecVideoPacket video_packet; ///< Input video packet to be decoded.
    DViewCtrl dview_ctrl;
    INT64 pkt_pts;
    INT64 pkt_dts;
    INT64 pkt_pos;
}VdecFrameConfig;

// Structure that can be used to get extended information about a decoded frame
typedef struct _VdecFrameInfo
{
    UINT32 frame_length;
    UINT32 frame_num;
    UINT32 vop_coded;

    UINT32 stride_y;
    UINT32 stride_uv;

    INT32 decode_status;
    INT32 decode_error;

    UINT8 disp_buf_id;
    UINT8 recon_buf_id;
    UINT8 ref_for_buf_id;
    UINT8 ref_back_buf_id;

    UINT32 disp_buf_prediction_type;
    UINT32 recon_buf_prediction_type;
    UINT32 for_buf_prediction_type;
    UINT32 bak_buf_prediction_type;

    UINT32 cur_frame_pts;   // presentation timestamp of the decoded frame.

    DViewCtrl disp_buf_dv_ctrl;
    DViewCtrl for_buf_dv_ctrl;
    DViewCtrl bak_buf_dv_ctrl;
}VdecFrameInfo;

// Structure used by display queue management
enum VdecFrmArrayType
{
	VDEC_FRM_ARRAY_MP = 0x01,
	VDEC_FRM_ARRAY_DV = 0x02,

	VDEC_FRM_ARRAY_INVALID = 0xff,
};

struct OutputFrmManager
{
	UINT32	display_index[2];

	enum VdecFrmArrayType de_last_request_frm_array[2];
	enum VdecFrmArrayType de_last_release_frm_array[2];
	//enum VdecFrmArrayType	de_using_frm_array[2];
	UINT8  de_last_request_idx[2];
	UINT8  de_last_release_idx[2];

	//UINT8	de_using_frm_idx[2];

    //UINT8	de_using_laf_idx;
	//UINT8	de_last_request_laf_idx;

	BOOL last_output_pic_released[2];
	INT32 de_last_release_poc[2];
	INT32 de_last_request_poc[2];

	UINT8 frm_number;
	UINT8 pip_frm_number;
};

struct RVdec_cfg
{
	int isRv8;
	int width;
	int height;
	void *extradata;
	int extradata_len;
	int rv_codec_id;
	unsigned long fb_base_addr;
	unsigned long fb_len;
	VdecInit *pDecInit;
};

struct vdec_decore_status
{
    UINT32 decode_status;
    UINT32 pic_width;
    UINT32 pic_height;
    UINT32 sar_width;
    UINT32 sar_height;
    UINT32 frame_rate;
    INT32 interlaced_frame;
    INT32 top_field_first;
    INT32 first_header_got;
    INT32 first_pic_showed;
    UINT32 frames_decoded;
    UINT32 frames_displayed;
    INT64 frame_last_pts;
    UINT32 buffer_size;
    UINT32 buffer_used;
    UINT32 decode_error;
    UINT32 decoder_feature;
    UINT32 under_run_cnt;
};

struct av_rect
{
	INT32 x;	// Horizontal start point.
	INT32 y;	// Vertical start point.
	INT32 w;	// Horizontal size.
	INT32 h;	// Vertical size.
};

#define AV_NOPTS_VALUE          (INT64)(0x8000000000000000)

#define AV_PKT_FLAG_KEY         0x00000001
#define AV_PKT_FLAG_CORRUPT     0x00000002 ///< The packet content is corrupted
#define AV_PKT_FLAG_EOS         0x20000000
#define AV_PKT_FLAG_ERROR       0x40000000
#define AV_PKT_FLAG_VIRTUAL     0x80000000

#define AV_BUFFER_HINTS_EOS     0x80000000

enum av_picture_type 
{
    AV_PICTURE_TYPE_I = 1, ///< Intra
    AV_PICTURE_TYPE_P,     ///< Predicted
    AV_PICTURE_TYPE_B,     ///< Bi-dir predicted
    AV_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG4
    AV_PICTURE_TYPE_SI,    ///< Switching Intra
    AV_PICTURE_TYPE_SP,    ///< Switching Predicted
    AV_PICTURE_TYPE_BI,    ///< BI type
};

/**
 * Pan Scan area.
 * This specifies the area which should be displayed.
 * Note there may be multiple such areas for one frame.
 */
struct av_panscan{
    /**
     * id
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    INT32 id;

    /**
     * width and height in 1/16 pel
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    INT32 width;
    INT32 height;

    /**
     * position of the top left corner in 1/16 pel for up to 3 fields/frames
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    INT16 position[3][2];
};

/**
 * rational number numerator/denominator
 */
struct av_rational
{
    INT32 num; ///< numerator
    INT32 den; ///< denominator
};

struct av_packet 
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
    INT64 pts;
    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    INT64 dts;
    UINT8 *data;
    INT32 size;
    INT32 stream_index;
    INT32 flags;
    void *side_data;
    INT32 side_data_elems;

    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    INT32 duration;
    void  (*destruct)(struct av_packet *);
    void  *priv;
    INT64 pos;                            ///< byte position in stream, -1 if unknown
    INT64 convergence_duration;
};

struct av_frame 
{
     /**
     * pointer to the picture planes.
     * This might be different from the first allocated byte
     * - encoding: 
     * - decoding: 
     */
    UINT8 *data[4];
    INT32 linesize[4];
    /**
     * pointer to the first allocated byte of the picture. Can be used in get_buffer/release_buffer.
     * This isn't used by libavcodec unless the default get/release_buffer() is used.
     * - encoding: 
     * - decoding: 
     */
    UINT8 *base[4];
    /**
     * 1 -> keyframe, 0-> not
     * - encoding: Set by libavcodec.
     * - decoding: Set by libavcodec.
     */
    INT32 key_frame;
    /**
     * Picture type of the frame, see ?_TYPE below.
     * - encoding: Set by libavcodec. for coded_picture (and set by user for input).
     * - decoding: Set by libavcodec.
     */
    enum av_picture_type pict_type;
    /**
     * presentation timestamp in time_base units (time when frame should be shown to user)
     * If AV_NOPTS_VALUE then frame_rate = 1/time_base will be assumed.
     * - encoding: MUST be set by user.
     * - decoding: Set by libavcodec.
     */
    INT64 pts;
    /**
     * picture number in bitstream order
     * - encoding: set by
     * - decoding: Set by libavcodec.
     */
    INT32 coded_picture_number;
    /**
     * picture number in display order
     * - encoding: set by
     * - decoding: Set by libavcodec.
     */
    INT32 display_picture_number;
    /**\
     * quality (between 1 (good) and FF_LAMBDA_MAX (bad)) 
     * - encoding: Set by libavcodec. for coded_picture (and set by user for input).
     * - decoding: Set by libavcodec.
     */
    INT32 quality; 
    /**
     * buffer age (1->was last buffer and dint change, 2->..., ...).
     * Set to INT_MAX if the buffer has not been used yet.
     * - encoding: unused
     * - decoding: MUST be set by get_buffer().
     */
    INT32 age;
    /**
     * is this picture used as reference
     * The values for this are the same as the MpegEncContext.picture_structure
     * variable, that is 1->top field, 2->bottom field, 3->frame/both fields.
     * Set to 4 for delayed, non-reference frames.
     * - encoding: unused
     * - decoding: Set by libavcodec. (before get_buffer() call)).
     */
    INT32 reference;
    /**
     * QP table
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    INT8 *qscale_table;
    /**
     * QP store stride
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    INT32 qstride;
    /**
     * mbskip_table[mb]>=1 if MB didn't change
     * stride= mb_width = (width+15)>>4
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    UINT8 *mbskip_table;
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
    UINT16 (*motion_val[2])[2];
    /**
     * macroblock type table
     * mb_type_base + mb_width + 2
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    UINT32 *mb_type;
    /**
     * log2 of the size of the block which a single vector in motion_val represents: 
     * (4->16x16, 3->8x8, 2-> 4x4, 1-> 2x2)
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    UINT8 motion_subsample_log2;
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
    INT64 error[4];
    /**
     * type of the buffer (to keep track of who has to deallocate data[*])
     * - encoding: Set by the one who allocates it.
     * - decoding: Set by the one who allocates it.
     * Note: User allocated (direct rendering) & internal buffers cannot coexist currently.
     */
    INT32 type;
    /**
     * When decoding, this signals how much the picture must be delayed.
     * extra_delay = repeat_pict / (2*fps)
     * - encoding: unused
     * - decoding: Set by libavcodec.
     */
    INT32 repeat_pict;
    /**
     * 
     */
    INT32 qscale_type;
    /**
     * The content of the picture is interlaced.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec. (default 0)
     */
    INT32 interlaced_frame;
    /**
     * If the content is interlaced, is top field displayed first.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    INT32 top_field_first;
    /**
     * Pan scan.
     * - encoding: Set by user.
     * - decoding: Set by libavcodec.
     */
    struct av_panscan *pan_scan;
    /**
     * Tell user application that palette has changed from previous frame.
     * - encoding: ??? (no palette-enabled encoder yet)
     * - decoding: Set by libavcodec. (default 0).
     */
    INT32 palette_has_changed;
    /**
     * codec suggestion on buffer type if != 0
     * - encoding: unused
     * - decoding: Set by libavcodec. (before get_buffer() call)).
     */
    INT32 buffer_hints;
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
    INT8 *ref_index[2];
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
    INT64 reordered_opaque;
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
    INT64 pkt_pts;
    /**
     * dts from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    INT64 pkt_dts;
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
    INT64 best_effort_timestamp;
    /**
     * reordered pos from the last AVPacket that has been input into the decoder
     * - encoding: unused
     * - decoding: Read by user.
     */
    INT64 pkt_pos;
    /**
     * reordered sample aspect ratio for the video frame, 0/1 if unknown\unspecified
     * - encoding: unused
     * - decoding: Read by user.
     */
    struct av_rational sample_aspect_ratio;
    /**
     * width and height of the video frame
     * - encoding: unused
     * - decoding: Read by user.
     */
    INT32 width, height;
    /**
     * format of the frame, -1 if unknown or unset
     * It should be cast to the corresponding enum (enum PixelFormat
     * for video, enum AVSampleFormat for audio)
     * - encoding: unused
     * - decoding: Read by user.
     */
    INT32 format;
};

struct audio_config
{
    INT32 decode_mode;
    INT32 sync_mode;
    INT32 sync_unit;
    INT32 deca_input_sbm;
    INT32 deca_output_sbm;
    INT32 snd_input_sbm;
    INT32 pcm_sbm;
    INT32 codec_id;
    INT32 bits_per_coded_sample;
    INT32 sample_rate;
    INT32 channels;
    INT32 bit_rate;
    UINT32 pcm_buf;
    UINT32 pcm_buf_size;
    INT32 block_align;
    UINT8 extra_data[512];
};

struct audio_frame
{
    UINT64 pts;
    UINT32 size;
    UINT32 pos;
    INT32 stc_id;
    UINT32 delay;
};

#ifdef __cplusplus
}
#endif

#endif
