#ifndef __ALI_SCART_MX9671_H__
#define __ALI_SCART_MX9671_H__

/*IO command relative parameters*/
#define SCART_PARAM_BASE		0
#define TV_MODE_RGB			(SCART_PARAM_BASE+1)
#define TV_MODE_CVBS			(SCART_PARAM_BASE+2)
#define TV_MODE_VCR			(SCART_PARAM_BASE+3)

#define ASPECT_4_3				(SCART_PARAM_BASE+4)
#define ASPECT_16_9				(SCART_PARAM_BASE+5)
#define ASPECT_INTERNAL		(SCART_PARAM_BASE+6)

#define SOURCE_TV_IN			(SCART_PARAM_BASE+7)
#define SOURCE_VCR_IN			(SCART_PARAM_BASE+8)
#define SOURCE_STB_IN			(SCART_PARAM_BASE+9)

#define SCART_STATE_VCR_IN		0x00000001
#define SCART_STATE_TV_IN		0x00000002


/*IO command list*/
#define SCART_IO_BASE			0x10000
#define SCART_TV_MODE			(SCART_IO_BASE+1)//TV_MODE_RGB:TV_MODE_CVBS:TV_MODE_VCR
#define SCART_TV_ASPECT		(SCART_IO_BASE+2)//ASPECT_4_3:ASPECT_16_9:ASPECT_INTERNAL
#define SCART_VCR_ASPECT		(SCART_IO_BASE+3)//ASPECT_4_3:ASPECT_16_9:ASPECT_INTERNAL
#define SCART_TV_SOURCE		(SCART_IO_BASE+4)//SOURCE_VCR_IN:SOURCE_STB_IN
#define SCART_VCR_SOURCE		(SCART_IO_BASE+5)//SOURCE_TV_IN:SOURCE_STB_IN
#define SCART_CHK_STATE		(SCART_IO_BASE+6)//Compare return value with SCART_STATE_VCR_IN
#define SCART_AUDIO_MUTE		(SCART_IO_BASE+7)// 0: MUTE, 1:RESUME
#define SCART_VCR_SB_OUT		(SCART_IO_BASE+8)// 0: VCR SB INPUT, 1:VCR SB OUTPUT
#define SCART_ENTRY_STADNBY		(SCART_IO_BASE+9)// 0: VCR SB INPUT, 1:VCR SB OUTPUT
#define SCART_REG_UPDATE		(SCART_IO_BASE+10)


#define MX_MAX_REG_LEN		18
#define ALI_SCART_ADDR    0x94
#define SCART_I2C_ID			1
#define ALI_SCART_DEVICE_NAME 	"ali_scart"


 struct scart_mx9671_private {
	void  *   priv_addr;	
	unsigned char reg_val[MX_MAX_REG_LEN];
	unsigned int base_addr;
	unsigned int i2c_type_id;
	struct cdev cdev;                  /* Char device structure		*/
};

#endif/*__SCART_MX9671_H__*/
