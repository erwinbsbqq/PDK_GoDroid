
/***************************************************************************************************
*    ALi Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:
*		hdmi_edid.c
*
*    Description:
*		ALi HDMI EDID Parser
*
*    History:
*	 	Date           Author        	Version     Reason
*	 	============	=============	=========	=================
*
*************************************************************************************************/
#include "hdmi_edid.h"
#include "hdmi_interrupt.h"
#include "hdmi_register.h"
#include "hdmi_proc.h"
//#include <../drivers/video/edid.h>


#define EDID_BYTE_TAG                   0x00
#define EDID_BYTE_REV_NUM               0x01
#define EDID_BYTE_TIMING_OFFSET         0x02
#define EDID_BYTE_THREE                 0x03    // [7]underscan, [6]audio, [5]YCbCr444, [4]YCbCr422 [3:0]Num_of_Native_DTDs
#define EDID_BYTE_START_DATA_BLOCK      0x04


#define EDID_CEA_BLOCK_TAG_AUDIO        1
#define EDID_CEA_BLOCK_TAG_VIDEO        2
#define EDID_CEA_BLOCK_TAG_VSDB         3
#define EDID_CEA_BLOCK_TAG_SPEAKER      4
#define EDID_CEA_BLOCK_TAG_VESA_DTC     5
#define EDID_CEA_BLOCK_TAG_EXTEND       7

#define EDID_AUD_SAMPLERATE_192KHZ      0x40
#define EDID_AUD_SAMPLERATE_176_4KHZ    0x20
#define EDID_AUD_SAMPLERATE_96KHZ       0x10
#define EDID_AUD_SAMPLERATE_88_2KHZ     0x08
#define EDID_AUD_SAMPLERATE_48KHZ       0x04
#define EDID_AUD_SAMPLERATE_44_1KHZ     0x02
#define EDID_AUD_SAMPLERATE_32KHZ       0x01


enum EDID_EXTENSION_TAG
{
	EDID_EXT_TAG_LCD_TIMING         = 0x01,
	EDID_EXT_TAG_CEA_861            = 0x02,
	EDID_EXT_TAG_EDID_2_0           = 0x20,
	EDID_EXT_TAG_COLOR_INFO         = 0x30,
	EDID_EXT_TAG_DVI_FEATURE        = 0x40,
	EDID_EXT_TAG_TOUCH_SCREEN_DATA  = 0x50,
	EDID_EXT_TAG_BLOCK_MAP          = 0xF0,
};


#define 	EDID_LENGTH			0x80
#define 	EDID_HEADER			0x00
#define 	EDID_HEADER_END     0x07


static unsigned char edid_v1_descriptor_flag[] = {0x00, 0x00};

extern HDMI_PRIVATE_DATA* hdmi_drv;
struct VENDOR_PRODUCT_BLOCK *vendor_product_block = NULL;
/***************************seiya add for further develop**********************************/
static struct DETAILED_TIMING_DESCRIPTOR	*detailed_timing_descriptor = NULL;
static struct SHORT_AUDIO_DESCRIPTOR		*short_audio_descriptor = NULL;
static struct SHORT_VIDEO_DESCRIPTOR		*short_video_descriptor = NULL;
static LIPSYNC_INFO_DESCRIPTOR_t 			lipsync_info_descriptor;

unsigned char monitor_name[14] = {0};
unsigned char hdmi_vic_list[36]={0};
unsigned char hdmi_update;

void parse_timing_description(unsigned char* dtd)
{
	DETAILED_TIMING_DESCRIPTOR_t  *detailed_descriptor = detailed_timing_descriptor;
	DETAILED_TIMING_DESCRIPTOR_t  *tmp=NULL;

	while (detailed_descriptor!=NULL)
	{
		tmp=detailed_descriptor;
		detailed_descriptor=detailed_descriptor->next;
	}

	detailed_descriptor=(DETAILED_TIMING_DESCRIPTOR_t *)kmalloc(sizeof(DETAILED_TIMING_DESCRIPTOR_t),GFP_KERNEL);

	if (NULL==detailed_descriptor)
		return ;
	memset(detailed_descriptor,0,(sizeof(DETAILED_TIMING_DESCRIPTOR_t)));

	detailed_descriptor->next = NULL;

	if (NULL!=tmp)
		tmp->next=detailed_descriptor;
	else
		detailed_timing_descriptor = detailed_descriptor;

	detailed_descriptor->pixel_clock_div_10000 = (dtd[1]<<8)|(dtd[0]);
	detailed_descriptor->h_active 		= (unsigned short) (( ((dtd[4]&0xF0) << 4) | dtd[2]) ); // 12 bits
	detailed_descriptor->h_blanking 	= (unsigned short) (( ((dtd[4]&0x0F) << 8) | dtd[3]) ); // 12 bits
	detailed_descriptor->v_active		= (unsigned short) (( ((dtd[7]&0xF0) << 4) | dtd[5]) ); // 12 bits
	detailed_descriptor->v_blanking 	= (unsigned short) (( ((dtd[7]&0x0F) << 8) | dtd[6]) ); // 12 bits	
	detailed_descriptor->h_sync_offset	= (unsigned short) (( ((dtd[11]&0xC0) << 2) | dtd[8]) ); // 10 bits
	detailed_descriptor->h_sync_pulse_w	= (unsigned short) (( ((dtd[11]&0x30) << 4) | dtd[9]) ); // 10 bits 	
	detailed_descriptor->v_sync_offset	=  (unsigned char) ((dtd[11]&0x0C)<<2) | ((dtd[10]&0xF0) >>4);	// 6 bits
	detailed_descriptor->v_sync_pulse_w	=  (unsigned char) ((dtd[11]&0x03)<<4) | ((dtd[10]&0x0F)       );	// 6 bits	
	detailed_descriptor->h_image_size 	= (unsigned short) (( ((dtd[14]&0xF0) << 4) | dtd[12]) ); // 12 bits
	detailed_descriptor->v_image_size 	= (unsigned short) (( ((dtd[14]&0x0F) << 8) | dtd[13]) ); // 12 bits	
	detailed_descriptor->h_border = dtd[15];	
	detailed_descriptor->v_border = dtd[16];	
	detailed_descriptor->interlaced = (dtd[17]>>7);

	EDID_DEBUG( "\tPixel clock (MHz): %d.%.2d\n", detailed_descriptor->pixel_clock_div_10000/100, detailed_descriptor->pixel_clock_div_10000%100);	
	EDID_DEBUG( "\tHorizontal active (pixels): %d\n", detailed_descriptor->h_active);	
	EDID_DEBUG( "\tHorizontal blanking (pixels):  %d\n", detailed_descriptor->h_blanking);
	EDID_DEBUG( "\tVertical active (lines):  %d\n", detailed_descriptor->v_active);	
	EDID_DEBUG( "\tVertical blanking (lines):  %d\n", detailed_descriptor->v_blanking);
	EDID_DEBUG( "\tHorizontal Sync offset (pixels):  %d\n", detailed_descriptor->h_sync_offset);	
	EDID_DEBUG( "\tHorizontal Sync pulse width (pixels):  %d\n", detailed_descriptor->h_sync_pulse_w);	
	EDID_DEBUG( "\tVertical Sync offset (lines):  %d\n", detailed_descriptor->v_sync_offset);	
	EDID_DEBUG( "\tVertical Sync pulse width (lines):  %d\n", detailed_descriptor->v_sync_pulse_w);
	EDID_DEBUG("\tHorizontal image size (mm): %d\n",detailed_descriptor->h_image_size);
	EDID_DEBUG("\tVertical image size (mm): %d\n",detailed_descriptor->v_image_size);
	EDID_DEBUG("\tScanning mode: %s\n", (detailed_descriptor->interlaced) ? "Interlaced":"Non-interlaced");	
	EDID_DEBUG("\tStereo: ");	
	switch(((dtd[17] & 0x60)>>4)|(dtd[17] & 0x01))
	{
		case 0:
		case 1:		EDID_DEBUG( "Normal display, no stereo.\n");								break;
		case 2:		EDID_DEBUG( "Field sequential stereo, right image when stereo sync. = 1\n");	break;
		case 3:		EDID_DEBUG( "Field sequential stereo, left image when stereo sync. = 1y\n");	break;		
		case 4:		EDID_DEBUG( "2-way interleaved stereo, right image on even lines\n");			break;
		case 5:		EDID_DEBUG( "2-way interleaved stereo, left image on even lines\n");			break;
		case 6:		EDID_DEBUG( "4-way interleaved stereo\n");								break;
		case 7:		EDID_DEBUG( "Side-by-Side interleaved stereo\n");							break;			
	}
	EDID_DEBUG("\tSync mode: ");	
	switch((dtd[17] & 0x18)>>3)
	{
		case 0:
			EDID_DEBUG( "Analog composite\n");								
			break;
		case 1:
			EDID_DEBUG( "Bipolar analog composite\n");								
			break;
		case 2:
			EDID_DEBUG( "Digital composite\n");								
			break;
		case 3:
			EDID_DEBUG( "Digital separate, ");	
			EDID_DEBUG( "VSYNC polarity %c, ", (dtd[17] & 0x04) ? '+':'-');	
			EDID_DEBUG( "HSYNC polarity %c\n", (dtd[17] & 0x02) ? '+':'-');
			break;			
	}
	
	return;
}

void hdmi_edid_parse_std(unsigned char *edid)
{
	unsigned int i, j;
	unsigned char* block;
	unsigned char edid_v1_header[] = {0x00, 0xff, 0xff, 0xff,0xff, 0xff, 0xff, 0x00};	
//	char monitor_alt_name[100];
	unsigned char checksum = 0;
	unsigned int detail_timing_block_type;
//	int ret = 0;
	bool edid_head_same;

    VENDOR_PRODUCT_BLOCK_t *vendor_product_id = vendor_product_block;
    vendor_product_id = (VENDOR_PRODUCT_BLOCK_t *)kmalloc(sizeof(VENDOR_PRODUCT_BLOCK_t), GFP_KERNEL);
    if(NULL == vendor_product_id)
        return;
    memset(vendor_product_id, 0, (sizeof(VENDOR_PRODUCT_BLOCK_t)));
    vendor_product_block = vendor_product_id;

	for( i = 0; i < EDID_LENGTH; i++)
		checksum += edid[ i ];

	if(memcmp( edid+EDID_HEADER, edid_v1_header, EDID_HEADER_END+1 ) == 0)
		edid_head_same = true;
	else
		edid_head_same = false;
		
	if(checksum != 0)
	{
		if(edid_head_same)
		{
			EDID_DEBUG( "%s in:EDID checksum failed - data is corrupt. Continuing anyway.\n" ,__FUNCTION__);
			return ;
		}
		
		checksum = 0;
		for(i=0; i<8;i++)
			checksum += edid_v1_header[i];

		for(i=8; i<EDID_LENGTH;i++)
			checksum += edid[ i ];
			
		if(checksum != 0)
		{
			EDID_DEBUG( "%s in:EDID checksum failed - data is corrupt. Continuing anyway.\n",__FUNCTION__);
			return ;
		}
  	}
	else if(!edid_head_same )
	{
		EDID_DEBUG( "%s in:8 byte pattern header (0x00, 0xff, 0xff, 0xff,0xff, 0xff, 0xff, 0x0) not match!!\n",__FUNCTION__);
		return ;
	}

    EDID_DEBUG("\nVendor and Product ID\n");	
	EDID_DEBUG("\tManufacturer Name: %c%c%c\n", (edid[0x08] >>2)+'A'-1, (((edid[0x08] <<3) | (edid[0x09] >>5)) & 0x1f)+'A'-1, (edid[0x09] & 0x1f)+'A'-1);	
	EDID_DEBUG("\tProduct Code: %d\n", edid[0x0B]<<8|edid[0x0A]);
	EDID_DEBUG("\tSerial Number: %d\n", edid[0x0F]<<24|edid[0x0E]<<16|edid[0x0D]<<8|edid[0x0C]);	
	EDID_DEBUG("\tWeek of Manufacture: %d\n", edid[0x10]);	
	EDID_DEBUG("\tYear of Manufacture: %d\n", edid[0x11]+1990);	
    vendor_product_id->manufacturer_name[0] = (edid[0x08] >>2)+'A'-1;
    vendor_product_id->manufacturer_name[1] = (((edid[0x08] <<3) |(edid[0x09] >>5)) & 0x1f)+'A'-1;
    vendor_product_id->manufacturer_name[2] = (edid[0x09] & 0x1f)+'A'-1;
    vendor_product_id->product_id = edid[0x0B]<<8|edid[0x0A];
    vendor_product_id->serial_number = edid[0x0F]<<24|edid[0x0E]<<16|edid[0x0D]<<8|edid[0x0C];
    vendor_product_id->week_manufacturer = edid[0x10];
    vendor_product_id->year_manufacturer = (edid[0x11]+1990);
    EDID_DEBUG("\nEDID Structure\n");	
	EDID_DEBUG( "\tVersion no.: %d\n" , edid[0x12]);
	EDID_DEBUG( "\tRevision no.: %d\n", edid[0x13]);
	
    EDID_DEBUG("\nBasic Display Parameters and Features\n");
	EDID_DEBUG( "\tVideo Input Signal Type: %s\n" , (edid[0x14]>>7) ? "Digital": "Analog");
	if(edid[0x14]>>7) 
	{
		// Digital
		EDID_DEBUG("\tVESA DFP 1.X default: %s\n", (edid[0x14]&0x01) ? "Compatible": "Not Compatible");	
	}
	
	EDID_DEBUG( "\tMax Horz Size (in cm): %d\n", edid[0x15]);
	EDID_DEBUG( "\tMax Vert Size (in cm): %d\n", edid[0x16]);
	if(edid[0x17] == 0xFF)
		EDID_DEBUG( "\tGamma Value: Not defined\n");
	else
		EDID_DEBUG( "\tGamma Value: %d.%d\n", (edid[0x17]+100)/100, (edid[0x17]+100)%100);
	EDID_DEBUG( "\tFeature Support (DPMS)\n");
	EDID_DEBUG( "\t\t Standby Mode: %s \n", (edid[0x18]&0x80) ? "Supported": "Not Supported");
	EDID_DEBUG( "\t\t Suspend Mode: %s \n", (edid[0x18]&0x40) ? "Supported": "Not Supported");
	EDID_DEBUG( "\t\t Active Off Mode: %s \n", (edid[0x18]&0x20) ? "Supported": "Not Supported");
	EDID_DEBUG( "\t\t Display Type: ");
	switch((edid[0x18]&0x18)>>3)
	{
		case 0:		EDID_DEBUG( "Monochrome display\n");		break;
		case 1:		EDID_DEBUG( "RGB color display\n");			break;
		case 2:		EDID_DEBUG( "Non-RGB multicolor display\n");	break;
		case 3:		EDID_DEBUG( "Undefined display\n");			break;			
	}
	EDID_DEBUG( "\t\t Color Space: %s \n", (edid[0x18]&0x04) ? "sRGB": "Alternate");
	EDID_DEBUG( "\t\t Preferred Timing: %s \n", (edid[0x18]&0x02) ? "1st DTD": "Non indicated");
	EDID_DEBUG( "\t\t GTF Timing: %s \n", (edid[0x18]&0x01) ? "Supported": "Not Supported");


	EDID_DEBUG("\nPhosphor or Filter Chromaticity\n");	
	EDID_DEBUG( "\tRed_x: 0.%.3d\n" 	,((edid[0x1B]<<2)  | ((edid[0x19]>>6)&0x03))*1000/1024);	
	EDID_DEBUG( "\tRed_y: 0.%.3d\n" 	,((edid[0x1C]<<2)  | ((edid[0x19]>>4)&0x03))*1000/1024);
	EDID_DEBUG( "\tGreen_x: 0.%.3d\n",((edid[0x1D]<<2)  | ((edid[0x19]>>2)&0x03))*1000/1024);
	EDID_DEBUG( "\tGreen_y: 0.%.3d\n",((edid[0x1E]<<2) | (edid[0x19]&0x03))*1000/1024);
	EDID_DEBUG( "\tBlue_x: 0.%.3d\n" 	,((edid[0x1F]<<2)  | ((edid[0x1A]>>6)&0x03))*1000/1024);
	EDID_DEBUG( "\tBlue_y: 0.%.3d\n" 	,((edid[0x20]<<2)  | ((edid[0x1A]>>4)&0x03))*1000/1024);
	EDID_DEBUG( "\tWhite_x: 0.%.3d\n" ,((edid[0x21]<<2)  | ((edid[0x1A]>>2)&0x03))*1000/1024);
	EDID_DEBUG( "\tWhite_y: 0.%.3d\n" ,((edid[0x22]<<2)  | (edid[0x1A]&0x03))*1000/1024);

	EDID_DEBUG("\nEstablished Timings\n");	
	EDID_DEBUG( "\tEstablished Timings I \n");
	if(edid[0x23]&0x80)	EDID_DEBUG( "\t\t 720 x 400 @ 70Hz IBM, VGA\n");
	if(edid[0x23]&0x40)	EDID_DEBUG( "\t\t 720 x 400 @ 88Hz IBM, XGA2\n");
	if(edid[0x23]&0x20)	EDID_DEBUG( "\t\t 640 x 480 @ 60Hz IBM, VGA\n");
	if(edid[0x23]&0x10)	EDID_DEBUG( "\t\t 640 x 480 @ 67Hz Apple, Mac II\n");
	if(edid[0x23]&0x08)	EDID_DEBUG( "\t\t 640 x 480 @ 72Hz VESA\n");
	if(edid[0x23]&0x04)	EDID_DEBUG( "\t\t 640 x 480 @ 75Hz VESA\n");
	if(edid[0x23]&0x02)	EDID_DEBUG( "\t\t 800 x 600 @ 56Hz VESA\n");
	if(edid[0x23]&0x01)	EDID_DEBUG( "\t\t 800 x 600 @ 60Hz VESA\n");	
	if(edid[0x23]== 0x00)	EDID_DEBUG( "\t\t None\n");	
	EDID_DEBUG( "\tEstablished Timings II \n");
	if(edid[0x24]&0x80)	EDID_DEBUG( "\t\t 800 x 600 @ 72Hz VESA\n");
	if(edid[0x24]&0x40)	EDID_DEBUG( "\t\t 800 x 600 @ 75Hz VESA\n");
	if(edid[0x24]&0x20)	EDID_DEBUG( "\t\t 832 x 624 @ 75Hz Apple, Mac II\n");
	if(edid[0x24]&0x10)	EDID_DEBUG( "\t\t 1024 x 768 @ 87Hz IBM\n");
	if(edid[0x24]&0x08)	EDID_DEBUG( "\t\t 1024 x 768 @ 60Hz VESA\n");
	if(edid[0x24]&0x04)	EDID_DEBUG( "\t\t 1024 x 768 @ 70Hz VESA\n");
	if(edid[0x24]&0x02)	EDID_DEBUG( "\t\t 1024 x 768 @ 75Hz VESA\n");
	if(edid[0x24]&0x01)	EDID_DEBUG( "\t\t 1280 x 1024 @ 75Hz VESA\n");	
	if(edid[0x24]== 0x00)	EDID_DEBUG( "\t\t None\n");		
	EDID_DEBUG( "\tManufacturer's Timings\n");
	if(edid[0x25]&0x80)	EDID_DEBUG( "\t\t 1152 x 870 @ 75Hz Apple, Mac II\n");
	else					EDID_DEBUG( "\t\t None\n");	
		
	EDID_DEBUG("\nStandard Timings\n");	
  	for(i = 0; i < 8; i++)
  	{
		EDID_DEBUG( "\tStandard Timing %d\n", i+1);	
		if(edid[0x26+i*2] == 0x01 && edid[0x26+i*2+1] == 0x01 )
			EDID_DEBUG( "\t\t Unused\n");	
		else
		{
			EDID_DEBUG( "\t\t Horizontal active pixels: %d\n", (edid[0x26+i*2]+31)*8);			
			EDID_DEBUG( "\t\t Image Aspect ratio: ");	
			switch((edid[0x26+i*2+1]&0xC0)>>6)
			{
				case 0:		EDID_DEBUG( "16:10\n");			break;
				case 1:		EDID_DEBUG( "4:3\n");			break;
				case 2:		EDID_DEBUG( "5:4\n");			break;
				case 3:		EDID_DEBUG( "16:9\n");			break;			
		}	
			EDID_DEBUG( "\t\t Refresh Rate: %d Hz\n", (edid[0x26+i*2+1]&0x3F)+60);			
		}
	}
	
  	block = edid + DETAILED_TIMING_DESCRIPTIONS_START;

	for( i = 0; i < TOTAL_NUM_DETAILED_TIMING_DESCRIPTIONS; i++, block += DETAILED_TIMING_DESCRIPTION_SIZE)
	{

		if(!memcmp(edid_v1_descriptor_flag, block, 2 ) )
		{
      			/* descriptor */
			if(block[2] != 0)
				detail_timing_block_type = UNKNOWN_DESCRIPTOR;
			else
				detail_timing_block_type = block[3];
		}
		else
			detail_timing_block_type = DETAILED_TIMING_BLOCK; /* detailed timing block */

		
		EDID_DEBUG("\nDetailed Timing Descriptor %d\n", i+1);
		switch(detail_timing_block_type)
		{
			case DETAILED_TIMING_BLOCK:
			 	parse_timing_description(block);
				break;
			case MONITOR_NAME:			 
				for(j = 5; j<18; j++)
                {     
                    monitor_name[j-5] = block[j];
					if(block[j] == 0x0A)
					{
						block[j] = 0x00;
                        monitor_name[j-5] = 0x00;
						break;
					}
                }
				EDID_DEBUG( "\tMonitor name: %s\n", block+5);	
				break;
			case MONITOR_LIMITS:
				EDID_DEBUG( "\tMin. Vertical rate (Hz): %d\n", block[5]);
				EDID_DEBUG( "\tMax. Vertical rate (Hz): %d\n", block[6]);			
				EDID_DEBUG( "\tMin. Horizontal rate (KHz): %d\n", block[7]);
				EDID_DEBUG( "\tMax. Horizontal rate (KHz): %d\n", block[8]);			
				EDID_DEBUG( "\tMax. Supported Pixel Clock rate (KHz): %d\n", block[9]*10);					
				if(block[10] == 0x00)
					EDID_DEBUG( "\tNo secondary timing supported\n");					
				else if(block[10] == 0x02)
				{
					EDID_DEBUG( "\tSecondary GTF  supported\n");						
				}
				break;	
			default:	
				break;				
	}

	}

    if(hdmi_drv->edid.block[0][0x7E] > MAX_EDID_BLOCKS_SUPPORT-1)
    {
        EDID_DEBUG("HDMI Driver: MAX_EDID_BLOCKS_SUPPORT(%d) not enouth!\n", MAX_EDID_BLOCKS_SUPPORT);  
        hdmi_drv->edid.number_of_extension_block =  MAX_EDID_BLOCKS_SUPPORT-1;
    }
    else
        hdmi_drv->edid.number_of_extension_block =  hdmi_drv->edid.block[0][0x7E];
	
	return ;
}

void parse_short_audio_description(unsigned char* sad)
{
	SHORT_AUDIO_DESCRIPTOR_t  *short_a_descriptor = short_audio_descriptor;
	SHORT_AUDIO_DESCRIPTOR_t  *tmp = NULL;

	while (short_a_descriptor!=NULL)
	{
		tmp=short_a_descriptor;
		short_a_descriptor = short_a_descriptor->next;
	}

	short_a_descriptor=(SHORT_AUDIO_DESCRIPTOR_t *)kmalloc(sizeof(SHORT_AUDIO_DESCRIPTOR_t),GFP_KERNEL);

	if (NULL==short_a_descriptor)
		return ;
	memset(short_a_descriptor,0,(sizeof(SHORT_AUDIO_DESCRIPTOR_t)));

	short_a_descriptor->next=NULL;

	if (NULL != tmp)
		tmp->next=short_a_descriptor;
	else
		short_audio_descriptor = short_a_descriptor;

	short_a_descriptor->audio_format_code = (sad[0] & 0x78)>>3;
	short_a_descriptor->max_num_audio_channels = (sad[0] & 0x7) + 1;
	short_a_descriptor->audio_sampling_rate = (sad[1] & 0x7F);
	if (0x0001 == short_a_descriptor->audio_format_code)//LPCM
	{
		switch(sad[2] & 0x7)
		{
			case 0x4:
				short_a_descriptor->max_audio_bit_rate = 24;
				break;
			case 0x2:
				short_a_descriptor->max_audio_bit_rate = 20;				
				break;
			case 0x1:
				short_a_descriptor->max_audio_bit_rate = 16;
				break;
			default:
				short_a_descriptor->max_audio_bit_rate = 0;
				break;
		}
	}
	else
	{
		short_a_descriptor->max_audio_bit_rate = (sad[2] << 3);
	}
 
	EDID_DEBUG("\tAudio format code: ");
	switch(short_a_descriptor->audio_format_code)
	{
		case 0: 		EDID_DEBUG("Reserved\n"); break;		
		case 1: 		EDID_DEBUG("Linear PCM (e.g., IEC60958)\n"); break;
		case 2: 		EDID_DEBUG("AC-3\n"); break;
		case 3: 		EDID_DEBUG("MPEG1 (Layers 1 & 2)\n"); break;
		case 4: 		EDID_DEBUG("MP3 (MPEG1 Layer 3)\n"); break;
		case 5: 		EDID_DEBUG("MPEG2 (multichannel)\n"); break;
		case 6: 		EDID_DEBUG("AAC\n"); break;
		case 7: 		EDID_DEBUG("DTS\n"); break;
		case 8: 		EDID_DEBUG("ATRAC\n"); break;	
		case 9: 		EDID_DEBUG("One Bit Audio\n"); break;
		case 10: 	EDID_DEBUG("Dolby Digital +\n"); break;
		case 11: 	EDID_DEBUG("DTS-HD\n"); break;
		case 12: 	EDID_DEBUG("MAT (MLP)\n"); break;		
		case 13: 	EDID_DEBUG("DST\n"); break;
		case 14: 	EDID_DEBUG("WMA Pro\n"); break;
		case 15: 	EDID_DEBUG("Reserved for audio format 15\n"); break;			
	}
	EDID_DEBUG("\t Maximum number of channels: %d\n", short_a_descriptor->max_num_audio_channels);	
	
	return;
}

void parse_short_video_description(unsigned char* svd)
{
	SHORT_VIDEO_DESCRIPTOR_t  *short_v_descriptor= short_video_descriptor;
	SHORT_VIDEO_DESCRIPTOR_t  *tmp=NULL;

	while (short_v_descriptor!=NULL)
	{
		tmp=short_v_descriptor;
		short_v_descriptor=short_v_descriptor->next;
	}

	short_v_descriptor=(SHORT_VIDEO_DESCRIPTOR_t *)kmalloc(sizeof(SHORT_VIDEO_DESCRIPTOR_t),GFP_KERNEL);

	if (NULL==short_v_descriptor)
		return;
	memset(short_v_descriptor,0,(sizeof(SHORT_VIDEO_DESCRIPTOR_t)));

	short_v_descriptor->next=NULL;

	if (NULL!=tmp)
		tmp->next=short_v_descriptor;
	else
		short_video_descriptor = short_v_descriptor;

	short_v_descriptor->native_indicator = (svd[0] & 0x80)>>7;
	short_v_descriptor->video_id_code = (svd[0] & 0x7F);
/*
 	EDID_DEBUG("\tVIC = %.2d, %s", short_v_descriptor->video_id_code, (short_v_descriptor->native_indicator) ? "Native": "     ");
	switch(short_v_descriptor->video_id_code)
	{
		case 1: 	EDID_DEBUG("\t640 x 480p  \t59.94Hz/60Hz \t4:3\n"); break;		
		case 2: 	EDID_DEBUG("\t720 x 480p  \t59.94Hz/60Hz \t4:3\n"); break;
		case 3: 	EDID_DEBUG("\t720 x 480p  \t59.94Hz/60Hz \t16:9\n"); break;
		case 4: 	EDID_DEBUG("\t1280 x 720p  \t59.94Hz/60Hz \t16:9\n"); break;	
		case 5: 	EDID_DEBUG("\t1920 x 1080i  \t59.94Hz/60Hz \t16:9\n"); break;
		case 6: 	EDID_DEBUG("\t720(1440) x 480i  \t59.94Hz/60Hz \t4:3\n"); break;
		case 7: 	EDID_DEBUG("\t720(1440) x 480i  \t59.94Hz/60Hz \t16:9\n"); break;	
		case 8: 	EDID_DEBUG("\t720(1440) x 240p  \t59.94Hz/60Hz \t4:3\n"); break;
		case 9: 	EDID_DEBUG("\t720(1440) x 240p  \t59.94Hz/60Hz \t16:9\n"); break;
		case 10: 	EDID_DEBUG("\t2880 x 480i  \t59.94Hz/60Hz \t4:3\n"); break;	
		case 11: 	EDID_DEBUG("\t2880 x 480i  \t59.94Hz/60Hz \t16:9\n"); break;	
		case 12: 	EDID_DEBUG("\t2880 x 240p  \t59.94Hz/60Hz \t4:3\n"); break;
		case 13: 	EDID_DEBUG("\t2880 x 240p  \t59.94Hz/60Hz \t16:9\n"); break;			
		case 14: 	EDID_DEBUG("\t1440 x 480p  \t59.94Hz/60Hz \t4:3\n"); break;	
		case 15: 	EDID_DEBUG("\t1440 x 480p  \t59.94Hz/60Hz \t16:9\n"); break;	
		case 16: 	EDID_DEBUG("\t1920 x 1080p  \t59.94Hz/60Hz \t16:9\n"); break;
		case 17: 	EDID_DEBUG("\t720 x 576p  \t50Hz \t4:3\n"); break;	
		case 18: 	EDID_DEBUG("\t720 x 576p  \t50Hz \t16:9\n"); break;
		case 19: 	EDID_DEBUG("\t1280 x 720p  \t50Hz \t16:9\n"); break;	
		case 20: 	EDID_DEBUG("\t1920 x 1080i  \t50Hz \t16:9\n"); break;
		case 21: 	EDID_DEBUG("\t720(1440) x 576i  \t50Hz \t4:3\n"); break;
		case 22: 	EDID_DEBUG("\t720(1440) x 576i  \t50Hz \t16:9\n"); break;
		case 23: 	EDID_DEBUG("\t720(1440) x 288p  \t50Hz \t4:3\n"); break;
		case 24: 	EDID_DEBUG("\t720(1440) x 288p  \t50Hz \t16:9\n"); break;
		case 25: 	EDID_DEBUG("\t2880 x 576i  \t50Hz \t4:3\n"); break;
		case 26: 	EDID_DEBUG("\t2880 x 576i  \t50Hz \t16:9\n"); break;
		case 27: 	EDID_DEBUG("\t2880 x 288p  \t50Hz \t4:3\n"); break;
		case 28: 	EDID_DEBUG("\t2880 x 288p  \t50Hz \t16:9\n"); break;
		case 29: 	EDID_DEBUG("\t1440 x 576p  \t50Hz \t4:3\n"); break;	
		case 30: 	EDID_DEBUG("\t1440 x 576p  \t50Hz \t16:9\n"); break;
		case 31: 	EDID_DEBUG("\t1920 x 1080p  \t50Hz \t16:9\n"); break;
		case 32: 	EDID_DEBUG("\t1920 x 1080p  \t23.97Hz/24Hz \t16:9\n"); break;
		case 33: 	EDID_DEBUG("\t1920 x 1080p  \t25Hz \t16:9\n"); break;		
		case 34: 	EDID_DEBUG("\t1920 x 1080p  \t23.97Hz/30Hz \t16:9\n"); break;
		case 35: 	EDID_DEBUG("\t2880 x 480p  \t59.94Hz/60Hz \t4:3\n"); break;	
		case 36: 	EDID_DEBUG("\t2880 x 480p  \t59.94Hz/60Hz \t16:9\n"); break;	

		default:	EDID_DEBUG("\n"); break;	

	}
*/
	return;
}

/******************************************************************************************/
void hdmi_edid_process_audio_data_block(unsigned char *data_block, int length)
{
    int i;
	int audio_format_code, max_number_of_channels, max_bit_rate = 0;
    unsigned char sample_rate_support, sample_length_support = 0;
    
    EDID_DEBUG("Audio Data Block:\n");
	
    for(i=0; i<length; i+=3)
    {
	    audio_format_code           = (data_block[i] & 0x78) >> 3;  // Byte#1  [6:3] Audio Format
	    max_number_of_channels      = (data_block[i] & 0x7) + 1;    // Byte#1  [2:0] Max Number of channels - 1 
	    sample_rate_support         = (data_block[i + 1] & 0x7F);   // Byte#2  [6:0] Sample Rate Support, 192K, 176.4K....        
	    if(audio_format_code == 0x01)
            sample_length_support   = data_block[i + 2];                // Byte#3  [2:0] Sample Size Support for Audio Code = 1 LPCM
        else if(audio_format_code >= 0x02 && audio_format_code <= 0x08)    
            max_bit_rate            = data_block[i + 2] * 8;            // Byte#3  Maximum bit rate divided by 8KHz for Audio Code 2 to 8

		parse_short_audio_description(&data_block[i]);
        switch(audio_format_code)
        {
     		case 0: 	EDID_DEBUG("\tAudio code: Reserved\n");                       break;
    		case 1: 	EDID_DEBUG("\tAudio code: Linear PCM (e.g., IEC60958)\n");    break;
    		case 2: 	EDID_DEBUG("\tAudio code: AC-3\n");                           break;
    		case 3: 	EDID_DEBUG("\tAudio code: MPEG1 (Layers 1 & 2)\n");           break;
    		case 4: 	EDID_DEBUG("\tAudio code: MP3 (MPEG1 Layer 3)\n");            break;
    		case 5: 	EDID_DEBUG("\tAudio code: MPEG2 (multichannel)\n");           break;
    		case 6: 	EDID_DEBUG("\tAudio code: AAC\n");                            break;
    		case 7: 	EDID_DEBUG("\tAudio code: DTS\n");                            break;
    		case 8: 	EDID_DEBUG("\tAudio code: ATRAC\n");                          break;
    		case 9: 	EDID_DEBUG("\tAudio code: One Bit Audio\n");                  break;
    		case 10: 	EDID_DEBUG("\tAudio code: Dolby Digital +\n");                break;
    		case 11: 	EDID_DEBUG("\tAudio code: DTS-HD\n");                         break;
    		case 12: 	EDID_DEBUG("\tAudio code: MAT (MLP)\n");                      break;
    		case 13: 	EDID_DEBUG("\tAudio code: DST\n");                            break;
    		case 14: 	EDID_DEBUG("\tAudio code: WMA Pro\n");                        break;
    		case 15: 	EDID_DEBUG("\tAudio code: Reserved for audio format 15\n");   break;
        }
        EDID_DEBUG("\tMaximum number of channels: %d\n", max_number_of_channels);
        EDID_DEBUG("\tSample Rate: %s%s%s%s%s%s%s\n",    (sample_rate_support & EDID_AUD_SAMPLERATE_192KHZ) ? "192KHz ":"",
                                                    (sample_rate_support & EDID_AUD_SAMPLERATE_176_4KHZ) ? "176.4KHz ":"",
                                                    (sample_rate_support & EDID_AUD_SAMPLERATE_96KHZ) ? "96KHz ":"",
                                                    (sample_rate_support & EDID_AUD_SAMPLERATE_88_2KHZ) ? "88.2KHz ":"",
                                                    (sample_rate_support & EDID_AUD_SAMPLERATE_48KHZ) ? "48KHz ":"",
                                                    (sample_rate_support & EDID_AUD_SAMPLERATE_44_1KHZ) ? "44.1KHz ":"",
                                                    (sample_rate_support & EDID_AUD_SAMPLERATE_32KHZ) ? "32KHz ":"" );
        if(audio_format_code == 0x01)       // LPCM
            EDID_DEBUG("\tSample Size: %s%s%s\n", (sample_length_support & 0x04) ? "24bit ":"",(sample_length_support & 0x02) ? "20bit ":"",(sample_length_support & 0x01) ? "16bit":"");
        else if(audio_format_code >= 0x02 && audio_format_code <= 0x08)  // Audio Codes 2 to 8
            EDID_DEBUG("\tMaximum bit rate: %d KHz\n", max_bit_rate*8);

    }
}

void hdmi_edid_process_video_data_block(unsigned char *data_block, int length)
{
    int i;
    bool native;
	int video_id_code;

    EDID_DEBUG("Video Data Block:\n");
    
    for(i=0; i<length; i++)
    {
    	parse_short_video_description(&data_block[i]);
        native = ((data_block[i] & 0x80) == 0x80);
	    video_id_code = data_block[i] & 0x7F;
		if (native)
		{
			EDID_DEBUG("[HDMI native]\tVIC = %.2d \n", video_id_code);
		}
        EDID_DEBUG("\tVIC = %.2d,%s", video_id_code, (native) ? "Native": "      ");

    	switch(video_id_code)
    	{
    		case 1: 	EDID_DEBUG("\t640x480p  \t59.94Hz/60Hz \t4:3\n"); break;
    		case 2: 	EDID_DEBUG("\t720x480p  \t59.94Hz/60Hz \t4:3\n"); break;
    		case 3: 	EDID_DEBUG("\t720x480p  \t59.94Hz/60Hz \t16:9\n"); break;
    		case 4: 	EDID_DEBUG("\t1280x720p  \t59.94Hz/60Hz \t16:9\n"); break;
    		case 5: 	EDID_DEBUG("\t1920x1080i  \t59.94Hz/60Hz \t16:9\n"); break;
    		case 6: 	EDID_DEBUG("\t720(1440)x480i  59.94Hz/60Hz \t4:3\n"); break;
    		case 7: 	EDID_DEBUG("\t720(1440)x480i  59.94Hz/60Hz \t16:9\n"); break;
    		case 8: 	EDID_DEBUG("\t720(1440)x240p  59.94Hz/60Hz \t4:3\n"); break;
    		case 9: 	EDID_DEBUG("\t720(1440)x240p  59.94Hz/60Hz \t16:9\n"); break;
    		case 10: 	EDID_DEBUG("\t2880x480i  \t59.94Hz/60Hz \t4:3\n"); break;
    		case 11: 	EDID_DEBUG("\t2880x480i  \t59.94Hz/60Hz \t16:9\n"); break;
    		case 12: 	EDID_DEBUG("\t2880x240p  \t59.94Hz/60Hz \t4:3\n"); break;
    		case 13: 	EDID_DEBUG("\t2880x240p  \t59.94Hz/60Hz \t16:9\n"); break;
    		case 14: 	EDID_DEBUG("\t1440x480p  \t59.94Hz/60Hz \t4:3\n"); break;
    		case 15: 	EDID_DEBUG("\t1440x480p  \t59.94Hz/60Hz \t16:9\n"); break;
    		case 16: 	EDID_DEBUG("\t1920x1080p  \t59.94Hz/60Hz \t16:9\n"); break;
    		case 17: 	EDID_DEBUG("\t720x576p  \t50Hz \t4:3\n"); break;
    		case 18: 	EDID_DEBUG("\t720x576p  \t50Hz \t16:9\n"); break;
    		case 19: 	EDID_DEBUG("\t1280x720p  \t50Hz \t16:9\n"); break;
    		case 20: 	EDID_DEBUG("\t1920x1080i  \t50Hz \t16:9\n"); break;
    		case 21: 	EDID_DEBUG("\t720(1440)x576i  50Hz \t4:3\n"); break;
    		case 22: 	EDID_DEBUG("\t720(1440)x576i  50Hz \t16:9\n"); break;
    		case 23: 	EDID_DEBUG("\t720(1440)x288p  50Hz \t4:3\n"); break;
    		case 24: 	EDID_DEBUG("\t720(1440)x288p  50Hz \t16:9\n"); break;
    		case 25: 	EDID_DEBUG("\t2880x576i  \t50Hz \t4:3\n"); break;
    		case 26: 	EDID_DEBUG("\t2880x576i  \t50Hz \t16:9\n"); break;
    		case 27: 	EDID_DEBUG("\t2880x288p  \t50Hz \t4:3\n"); break;
    		case 28: 	EDID_DEBUG("\t2880x288p  \t50Hz \t16:9\n"); break;
    		case 29: 	EDID_DEBUG("\t1440x576p  \t50Hz \t4:3\n"); break;
    		case 30: 	EDID_DEBUG("\t1440x576p  \t50Hz \t16:9\n"); break;
    		case 31: 	EDID_DEBUG("\t1920x1080p  \t50Hz \t16:9\n"); break;
    		case 32: 	EDID_DEBUG("\t1920x1080p  \t23.97Hz/24Hz \t16:9\n"); break;
    		case 33: 	EDID_DEBUG("\t1920x1080p  \t25Hz \t16:9\n"); break;
    		case 34: 	EDID_DEBUG("\t1920x1080p  \t23.97Hz/30Hz \t16:9\n"); break;
    		case 35: 	EDID_DEBUG("\t2880x480p  \t59.94Hz/60Hz \t4:3\n"); break;
    		case 36: 	EDID_DEBUG("\t2880x480p  \t59.94Hz/60Hz \t16:9\n"); break;
    		default:	EDID_DEBUG("\n"); break;
        }
    }

}

void hdmi_edid_process_VSDB_block(unsigned char *data_block, int length)
{
    unsigned int IEEE_Registration_Id; // 0x000C03 IEEE Registration ID, value belonging to HDMI Licensing.
    bool support_AI, DC_30bit, DC_36bit, DC_48bit, DC_Y444, DVI_Dual;
    unsigned int max_tmds_clock;
    unsigned int lipsync_video_latency, lipsync_audio_latency;
    unsigned int lipsync_interlaced_video_latency, lipsync_interlaced_audio_latency;
    
    EDID_DEBUG("VSDB Data Block:\n");

	hdmi_drv->edid.deep_color |= EDID_DEEPCOLOR_24;

	IEEE_Registration_Id    = ((data_block[2])<<16) | (data_block[1]<<8) | data_block[0];
	if(IEEE_Registration_Id == 0x000c03)
	{
		EDID_DEBUG("HDMI DEVICE:\n");
		hdmi_drv->edid.found_IEEE_reg_code = true;
	}
	else
	{
		// for CTS 7-33a
		//EDID_DEBUG("DVI DEVICE:\n");
		//hdmi_drv->edid.found_IEEE_reg_code = false;
		return;
	}
	hdmi_drv->edid.physical_address   = (data_block[3]<<8) | data_block[4];
	hdmi_drv->edid.support_3d = false;
    
	EDID_DEBUG("\tIEEE registration ID: 0x%.6x %s\n", IEEE_Registration_Id, (hdmi_drv->edid.found_IEEE_reg_code)? "(HDMI device)":"(DVI device)");				    
    EDID_DEBUG("\tPhysical address:  %.1x.%.1x.%.1x.%.1x\n", 
        (hdmi_drv->edid.physical_address&0xF000)>>12, (hdmi_drv->edid.physical_address&0x0F00)>>8, 
        (hdmi_drv->edid.physical_address&0x00F0)>>4, (hdmi_drv->edid.physical_address&0x000F)	   );	
					
	if(length >= 6) // Extension fields
	{
    	// Byte #6
        support_AI  = (data_block[5]&0x80) ?  true : false; // true if Sink supports at least one function  that uses info carred by ACP, ISRC1, ISRC2.
        DC_48bit    = (data_block[5]&0x40) ?  true : false; // true if Sink supports 48 bits/pixel (16 bits/color)
        DC_36bit    = (data_block[5]&0x20) ?  true : false; // true if Sink supports 36 bits/pixel (12 bits/color)
        DC_30bit    = (data_block[5]&0x10) ?  true : false; // true if Sink supports 30 bits/pixel (10 bits/color)
        DC_Y444     = (data_block[5]&0x08) ?  true : false; // true if Sink supports YCbCr 4:4:4 in Deep Color modes
        DVI_Dual    = (data_block[5]&0x01) ?  true : false; // true if Sink supports DVI dual-link operation
        
        if(support_AI)  EDID_DEBUG("\tSink Support ACP, ISRC1, ISRC2 packet\n");
        if(DC_48bit) 
        	{
        			hdmi_drv->edid.deep_color |= EDID_DEEPCOLOR_48;
			EDID_DEBUG("\tSink supports 48 bits/pixel [16 bits/color]\n");
        	}
        if(DC_36bit) 
        	{
        			hdmi_drv->edid.deep_color |= EDID_DEEPCOLOR_36;
			EDID_DEBUG("\tSink supports 36 bits/pixel [12 bits/color]\n");
	}
        if(DC_30bit)
        	{
        			hdmi_drv->edid.deep_color |= EDID_DEEPCOLOR_30;
			EDID_DEBUG("\tSink supports 30 bits/pixel [10 bits/color]\n");
        	}
        if(DC_Y444)  
        	{
        			hdmi_drv->edid.deep_color |= EDID_DEEPCOLOR_Y444;
			EDID_DEBUG("\tSink supports YCbCr 4:4:4 in Deep Color modes\n");
        	}
        if(DVI_Dual)    EDID_DEBUG("\tSink supports DVI dual-link operation\n");


        // Byte #7
        if(length >= 7)
        {
            max_tmds_clock = data_block[6]*5;
            EDID_DEBUG("\tMax TMDS Clock = %d MHz\n", max_tmds_clock);
        }

        // Byte #8 (Lip-Sync)
        if(length >= 8)
        {
            if( (data_block[7]&0x80) && (length >= 10))
            {
                lipsync_video_latency = (data_block[8] >=1 && data_block[8] <=251) ? (data_block[8]-1)*2 : 0;
                lipsync_audio_latency = (data_block[9] >=1 && data_block[9] <=251) ? (data_block[9]-1)*2 : 0;
                
                EDID_DEBUG("\tLipSync Information:\n"); 
                if(data_block[8] >=1 && data_block[8] <=251)    EDID_DEBUG("\t\tVideo Latency = %d ms\n",lipsync_video_latency);
                else                                            EDID_DEBUG("\t\tNo valid or unknownn video latency\n");
                if(data_block[9] >=1 && data_block[9] <=251)    EDID_DEBUG("\t\tAudio Latency = %d ms\n",lipsync_audio_latency);
                else                                            EDID_DEBUG("\t\tNo valid or unknownn audio latency\n");
                                           
                if( (data_block[7]&0x40) && (length >= 12)) // Interlaced Lip-Sync infomation
                {
                    lipsync_interlaced_video_latency = (data_block[10] >=1 && data_block[10] <=251) ? (data_block[10]-1)*2 : 0;
                    lipsync_interlaced_audio_latency = (data_block[11] >=1 && data_block[11] <=251) ? (data_block[11]-1)*2 : 0;
                    if(data_block[10] >=1 && data_block[10] <=251)  EDID_DEBUG("\t\tInterlaced Video Latency = %d ms\n",lipsync_interlaced_video_latency);
                    else                                            EDID_DEBUG("\t\tNo valid or unknownn interlaced video latency\n");                   
                    if(data_block[11] >=1 && data_block[11] <=251)  EDID_DEBUG("\t\tInterlaced Latency = %d ms\n",lipsync_interlaced_audio_latency);
                    else                                            EDID_DEBUG("\t\tNo valid or unknownn interlaced audio latency\n");
                    // Latency_present, I_Latency_present and HDMI_present 
                    if((data_block[7]&0x20) && (length >= 14))
                    {
                        EDID_DEBUG("\t3D Information:\n"); 
                        if(data_block[12]&0x80)
                        {
                            EDID_DEBUG("\t3D Present:\n"); 
                            hdmi_drv->edid.support_3d = true;
                        }
                    }
                }
                // only Latency_present and HDMI_present 
                else if((data_block[7]&0x20) && (length >= 12))
                {
                    EDID_DEBUG("\t3D Information:\n"); 
                    if(data_block[10]&0x80)
                    {
                        EDID_DEBUG("\t3D Present:\n"); 
                        hdmi_drv->edid.support_3d = true;
                    }
                }
            }
            // only HDMI_present
            else if((data_block[7]&0x20) && (length >= 10))
            {
                EDID_DEBUG("\t3D Information:\n"); 
                if(data_block[8]&0x80)
                {
                    EDID_DEBUG("\t3D Present:\n"); 
                    hdmi_drv->edid.support_3d = true;
                }
            }
        }
	}

}

void hdmi_edid_process_spk_alloc_block(unsigned char *data_block, int length)
{
    unsigned char speaker_allocation;

    if(length == 3)
    {
        EDID_DEBUG("Speaker Allocation Data Block:\n");
        speaker_allocation = data_block[0];
		hdmi_drv->edid.speaker_allocation = speaker_allocation;
        EDID_DEBUG("\t%s%s%s%s%s%s%s\n", (speaker_allocation & 0x40)?"RLC/RRC ":"",
                                     (speaker_allocation & 0x20)?"FLC/FRC ":"",
                                     (speaker_allocation & 0x10)?"RC ":"",
                                     (speaker_allocation & 0x08)?"RL/RR ":"",
                                     (speaker_allocation & 0x04)?"FC ":"",
                                     (speaker_allocation & 0x02)?"LFE ":"",
                                     (speaker_allocation & 0x01)?"FL/FR ":""  );
    }
	else
	{
		EDID_DEBUG("Not invalid speaker block!\n");
	}
}

void hdmi_edid_process_cea_data_block(unsigned char *data_block, unsigned char data_block_length)
{
    unsigned char tag_code = 0;
    unsigned int length, block_pos = 0;

    EDID_DEBUG("Data Block Collection\n" );

    while(block_pos < data_block_length)
    {
        tag_code = (data_block[block_pos] & 0xE0) >> 5; // [7:5] Tag Code
        length   = (data_block[block_pos] & 0x1F);      // [4:0] length = total number of bytes following this byte
        block_pos += 1;

        switch(tag_code)
        {
            case EDID_CEA_BLOCK_TAG_AUDIO:      hdmi_edid_process_audio_data_block(&data_block[block_pos], length);     break;
            case EDID_CEA_BLOCK_TAG_VIDEO:      hdmi_edid_process_video_data_block(&data_block[block_pos], length);     break;
            case EDID_CEA_BLOCK_TAG_VSDB:       hdmi_edid_process_VSDB_block(&data_block[block_pos], length);           break;
            case EDID_CEA_BLOCK_TAG_SPEAKER:    hdmi_edid_process_spk_alloc_block(&data_block[block_pos], length);      break;
            case EDID_CEA_BLOCK_TAG_VESA_DTC:   EDID_DEBUG("VESA DTC Data Block\n");                                        break;
            case EDID_CEA_BLOCK_TAG_EXTEND:     EDID_DEBUG("Use Extended Tag\n");                                           break;
            default:                            EDID_DEBUG("Unknown Data Block\n");                                         break;
        }
        block_pos += length;
    }

    return;
}

void hdmi_edid_process_detailed_timing_descriptor(unsigned char *dtd)
{
//    unsigned char* block;
    unsigned int pixel_clock_div_10000;
    unsigned int h_active, h_sync_offset, h_sync_plus_width, h_blanking;
    unsigned int v_active, v_sync_offset, v_sync_plus_width, v_blanking;
    unsigned int h_image_size, v_image_size, h_border, v_border;
	bool interlaced;
    
	pixel_clock_div_10000   = (dtd[1]<<8) | (dtd[0]);
	h_active 	            = ((dtd[4]&0xF0) << 4) | dtd[2];        // 12 bits
	h_blanking 	            = ((dtd[4]&0x0F) << 8) | dtd[3];        // 12 bits
	v_active	            = ((dtd[7]&0xF0) << 4) | dtd[5];        // 12 bits
	v_blanking 	            = ((dtd[7]&0x0F) << 8) | dtd[6];        // 12 bits
	h_sync_offset	        = ((dtd[11]&0xC0) << 2) | dtd[8];               // 10 bits
	h_sync_plus_width	    = ((dtd[11]&0x30) << 4) | dtd[9];               // 10 bits
	v_sync_offset	        = ((dtd[11]&0x0C) << 2) | ((dtd[10]&0xF0) >>4); // 6 bits
	v_sync_plus_width	    = ((dtd[11]&0x03) << 4) | (dtd[10]&0x0F);       // 6 bits
	h_image_size            = ((dtd[14]&0xF0) << 4) | dtd[12];      // 12 bits
	v_image_size            = ((dtd[14]&0x0F) << 8) | dtd[13];      // 12 bits
	h_border                = dtd[15];
	v_border                = dtd[16];
	interlaced              = (dtd[17]&0x80) ? true:false;
#if 0
    block = dtd;
	EDID_DEBUG("      %d MHz ",  PIXEL_CLOCK/1000000);
	EDID_DEBUG("%d %d %d %d ", H_ACTIVE, H_ACTIVE + H_SYNC_OFFSET, H_ACTIVE + H_SYNC_OFFSET + H_SYNC_WIDTH, H_ACTIVE + H_BLANKING);
	EDID_DEBUG("%d %d %d %d ", V_ACTIVE, V_ACTIVE + V_SYNC_OFFSET, V_ACTIVE + V_SYNC_OFFSET + V_SYNC_WIDTH, V_ACTIVE + V_BLANKING);
	EDID_DEBUG("%sHSync %sVSync\n", (HSYNC_POSITIVE) ? "+" : "-", (VSYNC_POSITIVE) ? "+" : "-");
#else
	EDID_DEBUG("\tPixel clock: %d.%.2dMHz\n", pixel_clock_div_10000/100, pixel_clock_div_10000%100);
	EDID_DEBUG("\tHorizontal active: %d pixels\n", h_active);
	EDID_DEBUG("\tHorizontal blanking: %d pixels\n", h_blanking);
	EDID_DEBUG("\tVertical active:  %d lines\n", v_active);
	EDID_DEBUG("\tVertical blanking: %d lines\n", v_blanking);
	EDID_DEBUG("\tHorizontal Sync offset: %d pixels\n", h_sync_offset);
	EDID_DEBUG("\tHorizontal Sync pulse width: %d pixels\n", h_sync_plus_width);
	EDID_DEBUG("\tVertical Sync offset: %d lines\n", v_sync_offset);
	EDID_DEBUG("\tVertical Sync pulse width: %d lines\n", v_sync_plus_width);
	EDID_DEBUG("\tHorizontal image size: %d mm\n", h_image_size);
	EDID_DEBUG("\tVertical image size: %d mm\n", v_image_size);
	EDID_DEBUG("\tScanning mode: %s\n", (interlaced) ? "Interlaced":"Non-interlaced");

	EDID_DEBUG("\tStereo: ");
	switch(((dtd[17] & 0x60)>>4)|(dtd[17] & 0x01))
	{
		case 0:
		case 1:		EDID_DEBUG( "Normal display, no stereo.\n");								    break;
		case 2:		EDID_DEBUG( "Field sequential stereo, right image when stereo sync. = 1\n");	break;
		case 3:		EDID_DEBUG( "Field sequential stereo, left image when stereo sync. = 1y\n");	break;
		case 4:		EDID_DEBUG( "2-way interleaved stereo, right image on even lines\n");			break;
		case 5:		EDID_DEBUG( "2-way interleaved stereo, left image on even lines\n");			break;
		case 6:		EDID_DEBUG( "4-way interleaved stereo\n");								    break;
		case 7:		EDID_DEBUG( "Side-by-Side interleaved stereo\n");							break;
	}
	EDID_DEBUG("\tSync mode: ");
	switch((dtd[17] & 0x18)>>3)
	{
		case 0:
			EDID_DEBUG( "Analog composite\n");
			break;
		case 1:
			EDID_DEBUG( "Bipolar analog composite\n");
			break;
		case 2:
			EDID_DEBUG( "Digital composite\n");
			break;
		case 3:
			EDID_DEBUG( "Digital separate, ");
			EDID_DEBUG( "VSYNC polarity %c, ", (dtd[17] & 0x04) ? '+':'-');
			EDID_DEBUG( "HSYNC polarity %c\n", (dtd[17] & 0x02) ? '+':'-');
			break;
	}
#endif

    return;
}


void hdmi_edid_parse_extension(unsigned char* edid)
{
    unsigned int i, j;

    EDID_DEBUG("Tag: %d\n", edid[EDID_BYTE_TAG]);
    switch(edid[EDID_BYTE_TAG])
    {
        case EDID_EXT_TAG_CEA_861:
        	EDID_DEBUG("Revision %d\n", edid[EDID_BYTE_REV_NUM]);
            hdmi_drv->edid.cea_version  = edid[EDID_BYTE_REV_NUM];

            // Byte 3 [7]underscan, [6]audio, [5]YCbCr444, [4]YCbCr422 [3:0]Num_of_Native_DTDs
        	hdmi_drv->edid.underscan 	        = ((edid[EDID_BYTE_THREE] & 0x80) == 0x80);
        	hdmi_drv->edid.support_basic_audio  = ((edid[EDID_BYTE_THREE] & 0x40) == 0x40);
        	hdmi_drv->edid.support_ycbcr444     = ((edid[EDID_BYTE_THREE] & 0x20) == 0x20);
        	hdmi_drv->edid.support_ycbcr422     = ((edid[EDID_BYTE_THREE] & 0x10) == 0x10);
        	hdmi_drv->edid.number_native_dtds   = edid[EDID_BYTE_THREE] & 0x0f;

        	EDID_DEBUG("Underscan: %s\n", (hdmi_drv->edid.underscan) ? "Supported":"Not Supported");
        	EDID_DEBUG("Basic audio: %s\n", (hdmi_drv->edid.support_basic_audio) ? "Supported":"Not Supported");
        	EDID_DEBUG("RGB and YCbCr444: %s\n", (hdmi_drv->edid.support_ycbcr444) ? "Supported":"Not Supported");
        	EDID_DEBUG("RGB and YCbCr422: %s\n", (hdmi_drv->edid.support_ycbcr422) ? "Supported":"Not Supported");
        	EDID_DEBUG("Total number of native DTDs: %d\n", hdmi_drv->edid.number_native_dtds);

            if(edid[EDID_BYTE_TIMING_OFFSET] != 4)
                hdmi_edid_process_cea_data_block(&edid[EDID_BYTE_START_DATA_BLOCK], edid[EDID_BYTE_TIMING_OFFSET]-4);

            if(edid[EDID_BYTE_TIMING_OFFSET] != 0)
            {
                for(i = edid[EDID_BYTE_TIMING_OFFSET], j=0; i+18<0x7F; i+=18, j++)
                {
                    EDID_DEBUG("Detailed Timing #%d\n", j);
                    hdmi_edid_process_detailed_timing_descriptor(&edid[i]);
                }
            }
            break;

        default:
            EDID_DEBUG("Unknown Extension Header: 0x%x\n", edid[0x00]);
            break;
    }
  
    return;
}

void edid_support_video_format(enum PicFmt *format)
{
		
	if (hdmi_drv != NULL)
	{
		if (hdmi_drv->edid.support_ycbcr444 == true)
			*format = YCBCR_444;		
		else if (hdmi_drv->edid.support_ycbcr422 == true)				
			*format  = YCBCR_422;
		else
			*format  = RGB_MODE1;
	}
	else
		*format  = RGB_MODE1;

	return ;
}

static enum HDMI_API_RES VideoIDCode_to_Resolution(unsigned char Video_id_code)
{
	int resolution = HDMI_RES_INVALID;
	
	switch(Video_id_code)
	{
		case 2:							//	720x480p 59.54Hz/60Hz 4:3
		case 3:							//	720x480p 59.54Hz/60Hz 16:9
			resolution = HDMI_RES_480P;
			break;
		case 4:							//	1280x720p 59.54Hz/60Hz 16:9
			resolution = HDMI_RES_720P_60;
			break;	
		case 5:							//	1920x1080i 59.54Hz/60Hz 16:9
			resolution = HDMI_RES_1080I_30;
			break;			
		case 6:							//	720(1440)x480i 59.54Hz/60Hz 4:3
		case 7:							//	720(1440)x480i 59.54Hz/60Hz 16:9
			resolution = HDMI_RES_480I;
			break;			
		case 16:
			resolution = HDMI_RES_1080P_60;
			break;			
		case 17:							//	720x576p 50Hz 4:3
		case 18:							//	720x576p 50Hz 16:9
			resolution = HDMI_RES_576P;
			break;		
		case 19:							//	1280x720p 50Hz 16:9
			resolution = HDMI_RES_720P_50;
			break;		
		case 20:							//	1920x1080i 50Hz 16:9
			resolution = HDMI_RES_1080I_25;
			break;		
		case 21:							//	720(1440)576i 50Hz 4:3
		case 22:							//	720(1440)576i 50Hz 16:9
			resolution = HDMI_RES_576I;
			break;
		case 31:
			resolution = HDMI_RES_1080P_50;
			break;			
		case 32:
			resolution = HDMI_RES_1080P_24;
			break;
		case 33:
			resolution = HDMI_RES_1080P_25;
			break;
		case 34:
			resolution = HDMI_RES_1080P_30;
			break;
		default:
			resolution = HDMI_RES_INVALID;			
			break;
	}
	return (enum HDMI_API_RES)resolution;
}

static enum HDMI_API_RES DTD_to_Resolution(DETAILED_TIMING_DESCRIPTOR_t *detailed_descriptor)
{
//	unsigned char video_scan_mode = 0;
	unsigned short vertical_resolution = 0;
	
	if(detailed_descriptor->interlaced == 1) // resolution is interlaced
	{
		vertical_resolution = (detailed_descriptor->v_active)*2;
		switch(detailed_descriptor->h_active)
		{
			case 720:
				if (480 == vertical_resolution)
					return HDMI_RES_480I;		
				else if (576 == vertical_resolution)
					return HDMI_RES_576I;	
				else
					return HDMI_RES_INVALID;	
			case 1920:
				if (1080 == vertical_resolution && 720 == detailed_descriptor->h_blanking && 528 == detailed_descriptor->h_sync_offset)
					return HDMI_RES_1080I_25;
				else if (1080 == vertical_resolution && 280 == detailed_descriptor->h_blanking && 88 == detailed_descriptor->h_sync_offset)
					return HDMI_RES_1080I_30;
				else
					return HDMI_RES_INVALID;					
			default:
				return HDMI_RES_INVALID;		
		}
	}
	else								
	{
	    vertical_resolution = (detailed_descriptor->v_active);
		switch(detailed_descriptor->h_active)
		{
			case 720: // SD resolution
				if (480 == vertical_resolution)
					return HDMI_RES_480P;			
				else if (576 == vertical_resolution)
					return HDMI_RES_576P;		
				else
					return HDMI_RES_INVALID;	
			case 1280:
				if (720 == vertical_resolution && 700 == detailed_descriptor->h_blanking && 440 == detailed_descriptor->h_sync_offset)
					return HDMI_RES_720P_50;			
				else if(720 == vertical_resolution && 370 == detailed_descriptor->h_blanking && 110 == detailed_descriptor->h_sync_offset)
					return HDMI_RES_720P_60;			
				else
					return HDMI_RES_INVALID;				
			case 1920:
				if (1080 == vertical_resolution && 280 == detailed_descriptor->h_blanking && 88 == detailed_descriptor->h_sync_offset)
				{
					if(detailed_descriptor->pixel_clock_div_10000 == 7424 )
						return HDMI_RES_1080P_30;		
					else if(detailed_descriptor->pixel_clock_div_10000 == 14850 )
						return HDMI_RES_1080P_60;		
						
				}	
				else	 if (1080 == vertical_resolution && 720 == detailed_descriptor->h_blanking && 528 == detailed_descriptor->h_sync_offset)
				{
					if(detailed_descriptor->pixel_clock_div_10000 == 7424 )
						return HDMI_RES_1080P_25;		
					else if(detailed_descriptor->pixel_clock_div_10000 == 14850 )
						return HDMI_RES_1080P_50;					
				}
				else if (1080 == vertical_resolution && 830 == detailed_descriptor->h_blanking && 638 == detailed_descriptor->h_sync_offset)
					return HDMI_RES_1080P_24;			
				else	
					return HDMI_RES_INVALID;
			default:
				return HDMI_RES_INVALID;			
		}		
	}
}

void edid_get_native_resolution(enum HDMI_API_RES *res)
{
	DETAILED_TIMING_DESCRIPTOR_t  *detailed_descriptor = detailed_timing_descriptor;
	SHORT_VIDEO_DESCRIPTOR_t *short_v_descriptor = short_video_descriptor;
//	unsigned char video_scan_mode = 0;
//	unsigned short vertical_resolution = 0;
	unsigned char i = 0, index = 0;
	enum HDMI_API_RES best_resolution = HDMI_RES_INVALID;
	
	*res = HDMI_RES_INVALID;
	hdmi_drv->edid.native_res_index = 0xFF;
	
	while(short_v_descriptor != NULL)
	{
		hdmi_drv->edid.resolution_support[index] = VideoIDCode_to_Resolution(short_v_descriptor->video_id_code);

		if(hdmi_drv->edid.resolution_support[index] != HDMI_RES_INVALID)
		{
			for(i=0; i<=index; i++)
				if(hdmi_drv->edid.resolution_support[i] == hdmi_drv->edid.resolution_support[index])
					break;
			if(i == index)	
			{
				if(short_v_descriptor->native_indicator && hdmi_drv->edid.native_res_index == 0xFF)	
				{					
					hdmi_drv->edid.native_res_index = index;
					*res = hdmi_drv->edid.resolution_support[index];				
				}		
				else				
					if(hdmi_drv->edid.resolution_support[index] > best_resolution)
						best_resolution = hdmi_drv->edid.resolution_support[index];	
					
				if(index == 13)				
				{
					if ( *res != HDMI_RES_INVALID)	
						return;			
					else if(best_resolution  != HDMI_RES_INVALID)
					{
						*res  = best_resolution;
						return;
					}
				}
				index++;				
			}	
			else				
				hdmi_drv->edid.resolution_support[index] = HDMI_RES_INVALID;		
		}
		short_v_descriptor = short_v_descriptor->next;		
	}	

	while(detailed_descriptor != NULL )
	{
		hdmi_drv->edid.resolution_support[index] = DTD_to_Resolution(detailed_descriptor);
		if(hdmi_drv->edid.resolution_support[index] != HDMI_RES_INVALID)
		{		
			for(i =0;i<=index; i++)
				if(hdmi_drv->edid.resolution_support[i] == hdmi_drv->edid.resolution_support[index])
					break;
			if(i == index)
			{
				if(hdmi_drv->edid.resolution_support[index] > best_resolution)
					best_resolution = hdmi_drv->edid.resolution_support[index];
		
				if(index == 13)
				{	
					if ( *res != HDMI_RES_INVALID)	
						return;			
					else if(best_resolution  != HDMI_RES_INVALID)	
					{
						*res  = best_resolution;
						return;
					}	
				}
				index++;
			}		
			else
				hdmi_drv->edid.resolution_support[index] = HDMI_RES_INVALID;
		}		
		detailed_descriptor = detailed_descriptor->next;
	}
	
	if ( *res != HDMI_RES_INVALID)	
		return;
	else if(best_resolution  != HDMI_RES_INVALID)
	{
		*res  = best_resolution;
		EDID_DEBUG("selection best_resolution %d\n", best_resolution);
		return;
	}
	else
	{
		EDID_DEBUG("selection doesn't match in any descriptors\n");
		return;
	}	
}


short edid_get_lipsync_audio_delay(bool interlaced_video)
{
	LIPSYNC_INFO_DESCRIPTOR_t* lipsync_info = &lipsync_info_descriptor;
	
	if((lipsync_info->interlaced_latency_present == true) && (interlaced_video	==	true)) 	// Interlaced Lipsync information present		
		return lipsync_info->interlaced_video_latency - lipsync_info->interlaced_audio_latency;
	else																						// Progress Video or interlaced lipsync not present.	
		return lipsync_info->video_latency - lipsync_info->audio_latency;
}


void edid_get_prefer_audio_out(unsigned short *aud_fmt_code)
{
	SHORT_AUDIO_DESCRIPTOR_t  *short_a_descriptor = short_audio_descriptor;
//	unsigned int i = 0;	

	*aud_fmt_code = 0x00;
	if (short_a_descriptor == NULL)
		return ;
	
	while(short_a_descriptor != NULL)
	{
		if (short_a_descriptor->audio_format_code != 0)	//reserved field
			*aud_fmt_code = *aud_fmt_code | (1 << (short_a_descriptor->audio_format_code-1));

		EDID_DEBUG("edid_get_prefer_audio_out aud_fmt_code %x\n", short_a_descriptor->audio_format_code);

		short_a_descriptor = short_a_descriptor->next;		
	}
	return;
}

void edid_get_max_channel_number(unsigned char *aud_max_ch_num)
{
	SHORT_AUDIO_DESCRIPTOR_t  *short_a_descriptor= short_audio_descriptor;
	*aud_max_ch_num = 0x00;
	if (short_a_descriptor == NULL)
		return;
	while(short_a_descriptor != NULL)
	{
		if (short_a_descriptor->audio_format_code == 1 && short_a_descriptor->max_num_audio_channels > *aud_max_ch_num)
		{
			*aud_max_ch_num = short_a_descriptor->max_num_audio_channels;			
		}	
		short_a_descriptor = short_a_descriptor->next;		
	}
	return;
}


bool hdmi_edid_chk_intface(void)
{
	if(hdmi_drv)
		return hdmi_drv->edid.found_IEEE_reg_code;
	return false;
}

bool supports_AI_inVSDB(void)
{
	if(hdmi_drv)
		return hdmi_drv->edid.support_AI;
	return false;
}

void hdmi_edid_clear(void)
{
	DETAILED_TIMING_DESCRIPTOR_t *detailed_descriptor;
	SHORT_AUDIO_DESCRIPTOR_t *short_a_descriptor;
	SHORT_VIDEO_DESCRIPTOR_t *short_v_descriptor;
	VENDOR_PRODUCT_BLOCK_t *vendor_p_block;
	
	while(detailed_timing_descriptor != NULL)
	{
		detailed_descriptor = detailed_timing_descriptor->next;
		kfree(detailed_timing_descriptor);
		detailed_timing_descriptor = detailed_descriptor;
	}

	while(short_audio_descriptor != NULL)
	{
		EDID_DEBUG("hdmi_edid_clear \n");
		short_a_descriptor = short_audio_descriptor->next;
		kfree(short_audio_descriptor);
		short_audio_descriptor = short_a_descriptor;
	}

	while(short_video_descriptor != NULL)
	{
		short_v_descriptor = short_video_descriptor->next;
		kfree(short_video_descriptor);
		short_video_descriptor = short_v_descriptor;
	}
	
	while(vendor_product_block != NULL)
	{
		vendor_p_block = vendor_product_block->next;
		kfree(vendor_product_block);
		vendor_product_block = vendor_p_block;
	}
	
	memset(&lipsync_info_descriptor, 0, sizeof(LIPSYNC_INFO_DESCRIPTOR_t));	
}

