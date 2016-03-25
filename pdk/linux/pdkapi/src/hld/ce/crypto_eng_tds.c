#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <hld/adr_hld_dev.h>
#include <hld/crypto/adr_crypto.h>

#ifndef ALI_CE_HLD_TDS_DBG
#define ALI_CE_HLD_TDS_PRF(...) do{}while(0)
#else
#define ALI_CE_HLD_TDS_PRF(fmt, args...)  \
			do { \
					ADR_DBG_PRINT(CE_HLD_TDS, fmt, ##args); \
			} while(0)
#endif

int ce_api_attach( void )
{
	int ret = RET_FAILURE;
	pCE_DEVICE pCeDev = NULL;
	int ce_fd=-1;
	/*
	pCeDev = dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	if ( pCeDev )
	{
		ALI_CE_HLD_TDS_PRF("CE hld warning: pCeDev has already been attached!\n"); 
		return RET_SUCCESS;
	}
	*/
	ce_fd = open(ALI_CE_LINUX_DEV_PATH, O_RDWR);
	if (ce_fd < 0)
	{
		ALI_CE_HLD_TDS_PRF("CE hld error: open ce_dev failed\n"); 
		return (RET_FAILURE);
	}
	
	pCeDev = dev_alloc(ALI_CE_HLD_DEV_NAME, HLD_DEV_TYPE_CE, sizeof(CE_DEVICE));
	if ( pCeDev == NULL )
	{
		ALI_CE_HLD_TDS_PRF("CE hld error:pCeDev is NULL\n"); 
		close(ce_fd);
		return RET_FAILURE;
	}     

	pCeDev->fd = ce_fd;
	if ( dev_register(pCeDev) != RET_SUCCESS )
	{
		dev_free(pCeDev);
		close(ce_fd);
		ALI_CE_HLD_TDS_PRF("CE hld error: dev_register\n"); 
		return RET_FAILURE;
	}
	else
	{
		ALI_CE_HLD_TDS_PRF("CE attach success! pCeDev->fd=%d\n", pCeDev->fd);
	}
	
	return RET_SUCCESS;
}

int ce_api_detach( void )
{
	pCE_DEVICE pCeDev = dev_get_by_id(HLD_DEV_TYPE_CE, 0);

	if(NULL == pCeDev)
	{
		ALI_CE_HLD_TDS_PRF("Err: pCeDev is NULL\n");
		return RET_FAILURE;
	}
	
	if(pCeDev->fd > 0)
	{
		close(pCeDev->fd);
	}
	
	dev_free(pCeDev);

	ALI_CE_HLD_TDS_PRF("CE detached\n");  
	return RET_SUCCESS;
}