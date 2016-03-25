#include <common.h>
#include "baseparams.h"

#include <boot_common.h>
#include <ali/sys_parameters.h>

#define BASEPARAMS_HEADER 12
#define SIG_LEN     0x100

extern unsigned char g_uk_pos;
static struct base_params *baseparams=NULL;

struct base_params * get_baseparams()
{
	int ret;
	const char *partName = "baseparams";
	struct base_params *baseparams_tmp_buff = NULL;
    unsigned char *p_temp = NULL;
    unsigned int rsa_data_len = 0;
	
	if (baseparams)
		return baseparams;		
	rsa_data_len = ((sizeof(struct base_params)+BASEPARAMS_HEADER+0x20-1) & ~(0x20-1)) + SIG_LEN;
	baseparams = (struct base_params*)malloc(sizeof(struct base_params));
	
	baseparams_tmp_buff = (struct base_params*)malloc(rsa_data_len);
        p_temp = baseparams_tmp_buff;
	
#if defined(CONFIG_NAND_ALI) 
	//ret = load_part_sector(partName, (u_char *)baseparams_tmp_buff, sizeof(struct base_params));
	ret = load_part_sector(partName, (u_char *)baseparams_tmp_buff, rsa_data_len);
#endif

 #if defined(CONFIG_ALI_MMC)      //emmc
	ret = load_part_sector_emmc(partName, (u_char *)baseparams_tmp_buff, rsa_data_len);
#endif
/*
    aes_cbc_decrypt_ram_chunk(g_uk_pos,baseparams, baseparams_tmp_buff, sizeof(struct base_params));

	if( version_check(CHUNKID_SEECODE,(UINT32)baseparams_tmp_buff,sizeof(struct base_params)) != 0){
		printf("baseparams  version check failed\n");
		return NULL;
	} 
	*/
#if 1
	if(test_rsa_ram((UINT32)baseparams_tmp_buff, rsa_data_len) != 0)
	{
		printf("baseparams  rsa verify error\n");
		return NULL;
	}
#endif
       memcpy(baseparams, (void *)(p_temp+BASEPARAMS_HEADER), sizeof(struct base_params));
	if (ret < 0)
	{
		printf("get baseparams fail.\n");
		free(baseparams);
		return NULL;
	}
	free(baseparams_tmp_buff);
	return baseparams;	
}

unsigned int get_avinfo(unsigned char **avinfo)
{
	struct base_params *baseparams = get_baseparams();

	if (baseparams == NULL)
	{
		printf("get avinfo fail.\n");		
		*avinfo = NULL;
		return 0;
	}

	*avinfo = &baseparams->avinfo;
	return sizeof(ADF_BOOT_AVINFO);
}

