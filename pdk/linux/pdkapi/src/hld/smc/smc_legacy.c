
#include <string.h>
#include <hld/smc/adr_smc.h>


INT32 smc_attach(int dev_id, int use_default_cfg, struct smc_dev_cfg *cfg)
{
	struct smc_dev_cfg config_param;

	if (NULL != cfg)
	{
		cfg->use_default_cfg = use_default_cfg;
		return smc_dev_attach(dev_id, cfg);
		
	}
	else
	{
		memset(&config_param, 0x00, sizeof(config_param));
		config_param.use_default_cfg = use_default_cfg;
		return smc_dev_attach(dev_id, &config_param);
	}
}



