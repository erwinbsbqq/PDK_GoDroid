#include <common.h>
#include "baseparams.h"

static struct base_params *baseparams=NULL;

struct base_params * get_baseparams()
{
	int ret;
	const char *partName = "baseparams";

	if (baseparams)
		return baseparams;		
	
	baseparams = (struct base_params*)malloc(sizeof(struct base_params));
	memset(baseparams, 0, sizeof(struct base_params));

	ret = load_part_sector(partName, (u_char *)baseparams, sizeof(struct base_params));
	if (ret < 0)
	{
		printf("get baseparams fail.\n");
		free(baseparams);
		return NULL;
	}
	
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

