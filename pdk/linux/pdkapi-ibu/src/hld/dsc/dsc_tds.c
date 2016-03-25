#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>

#ifndef ALI_DSC_HLD_TDS_DBG
#define ALI_DSC_TDS_HLD_PRF(...) do{}while(0)
#else
#define ALI_DSC_TDS_HLD_PRF(fmt, arg...) \
    do { \
        printf("DSC_HLD_TDS: In %s "fmt, __func__, ##arg); \
    } while (0)
#endif


static char dsc_dev_name[] = 
{
	"ALI_DSC_0"
};

static char sha_dev_name[] = 
{
	"ALI_SHA_0"
};

static char des_dev_name[] =
{
    "ALI_DES_0"
};
static char aes_dev_name[] =
{
    "ALI_AES_0"
};
static char csa_dev_name[] =
{
    "ALI_CSA_0"
};


int des_api_attach( void )
{
	int des_fd = -1;
	pDES_DEV pDesDev = NULL;
	UINT32 i = 0;
	int ret = -1;
	struct dsc_see_dev_hld see_dev_hld;

	des_dev_name[strlen(des_dev_name)-1] += '0'; 
	des_fd = open(ALI_DES_LINUX_DEV_PATH, O_RDWR);
	if(des_fd < 0)
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: open des_fd\n"); 
		return des_fd;
	}

	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		des_dev_name[strlen(des_dev_name)-1] += (char)1; 
		see_dev_hld.dsc_dev_id = i;
		ret = ioctl(des_fd, IO_GET_DEV_HLD, (UINT32)&see_dev_hld);
		if(ret < 0)
		{
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: IO_GET_DEV_HLD\n");
			close(des_fd);
			return RET_FAILURE;
		}
		
		pDesDev = dev_alloc(des_dev_name, HLD_DEV_TYPE_DES, sizeof(DES_DEV));
		if ( pDesDev == NULL )
		{
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_alloc\n");
			close(des_fd);
			return RET_FAILURE;
		}

		pDesDev->id_number = i;
		pDesDev->fd = des_fd;
		pDesDev->priv = (void *)see_dev_hld.dsc_dev_hld;
		if ( dev_register(pDesDev) != RET_SUCCESS )
		{
			dev_free(pDesDev);
			close(des_fd);
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_register\n"); 
			return RET_FAILURE;
		}
		else
		{
			ALI_DSC_TDS_HLD_PRF("DES attach success, pDesDev->fd=%d\n",des_fd);
		}
		
	}
	return RET_SUCCESS;
}

static int des_api_detach(void)
{
	pDES_DEV pDesDev = NULL;
	UINT32 i = 0;
	int des_fd = -1;
	
	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		pDesDev = dev_get_by_id(HLD_DEV_TYPE_DES, i);
		if ( pDesDev == NULL )
		{
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: pDesDev is NULL%d\n"); 
			return RET_FAILURE ;
		}

		des_fd = pDesDev->fd;
		dev_free(pDesDev);
	}
	
	if(des_fd > 0)
	{
		close(des_fd);
	}

	ALI_DSC_TDS_HLD_PRF("des_api_detached\n"); 
	return RET_SUCCESS;    
}

int sha_api_attach( void )
{
	int sha_fd = -1;
	pSHA_DEV pShaDev = NULL;
	UINT32 i=0;
	int ret = -1;
	struct dsc_see_dev_hld see_dev_hld;

	sha_dev_name[strlen(sha_dev_name)-1] = '0';
	sha_fd = open(ALI_SHA_LINUX_DEV_PATH, O_RDWR);
	if(sha_fd < 0)
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: open sha_fd\n"); 
		return sha_fd;
	}
	
	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		sha_dev_name[strlen(sha_dev_name)-1] += (char)1;
		see_dev_hld.dsc_dev_id = i;
		ret = ioctl(sha_fd, IO_GET_DEV_HLD, (UINT32)&see_dev_hld);
		if(ret < 0)
		{
			close(sha_fd);
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: IO_GET_DEV_HLD\n"); 
			return RET_FAILURE;
		}
		
		pShaDev = dev_alloc(sha_dev_name, HLD_DEV_TYPE_SHA, sizeof(SHA_DEV));
		if ( pShaDev == NULL )
		{
			close(sha_fd);
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_alloc\n"); 
			return RET_FAILURE;
		}

		pShaDev->id_number = i;
		pShaDev->fd = sha_fd;
		pShaDev->priv = (void *)see_dev_hld.dsc_dev_hld;
		if ( dev_register(pShaDev) != RET_SUCCESS )
		{
			close(sha_fd);
			dev_free(pShaDev);
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_register\n"); 
			return RET_FAILURE;
		}
		else
		{
			ALI_DSC_TDS_HLD_PRF("SHA attach success, pShaDev->fd=%d\n",sha_fd);
		}
	}
	
	return RET_SUCCESS;  
}

static int sha_api_detach(void)
{
	pSHA_DEV pShaDev = NULL;
	UINT32 i = 0;
	int sha_fd = -1;
	
	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		pShaDev = dev_get_by_id(HLD_DEV_TYPE_SHA, i);
		if ( pShaDev == NULL )
		{
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: pShaDev is NULL\n"); 
			return RET_FAILURE ;
		}

		sha_fd = pShaDev->fd;
		dev_free(pShaDev);
	}

	if(sha_fd > 0)
	{
		close(sha_fd);
	}
	ALI_DSC_TDS_HLD_PRF("sha_api_detached\n"); 
	return RET_SUCCESS;    
}

int aes_api_attach( void )
{
	int aes_fd = -1;
	pAES_DEV pAesDev = NULL;
	UINT32 i=0;
	int ret = -1;
	struct dsc_see_dev_hld see_dev_hld;
	
	aes_dev_name[strlen(aes_dev_name)-1] = '0';

	aes_fd = open(ALI_AES_LINUX_DEV_PATH, O_RDWR);
	if(aes_fd < 0)
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: open aes_fd\n"); 
		return aes_fd;
	}
		
	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		aes_dev_name[strlen(aes_dev_name)-1] += (char)1; 
		see_dev_hld.dsc_dev_id = i;
		ret = ioctl(aes_fd, IO_GET_DEV_HLD, (UINT32)&see_dev_hld);
		if(ret < 0)
		{
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: IO_GET_DEV_HLD\n"); 
			close(aes_fd);
			return RET_FAILURE;
		}
		
		pAesDev = dev_alloc(aes_dev_name, HLD_DEV_TYPE_AES, sizeof(AES_DEV));
		if ( pAesDev == NULL )
		{
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_alloc\n"); 
			close(aes_fd);
			return RET_FAILURE ;
		}

		pAesDev->id_number = i;
		pAesDev->fd = aes_fd;
		pAesDev->priv = (void *) see_dev_hld.dsc_dev_hld;
		if ( dev_register(pAesDev) != RET_SUCCESS )
		{
			close(aes_fd);
			dev_free(pAesDev);
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_register\n"); 
			return RET_FAILURE;
		}
		else
		{
			ALI_DSC_TDS_HLD_PRF("AES attach success, pAesDev->fd=%d\n",aes_fd);
		}
		
	}
	
	return RET_SUCCESS;    
}

static int aes_api_detach(void)
{
	pAES_DEV pAesDev = NULL;
	UINT32 i=0;
	int aes_fd = -1;
	
	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		pAesDev = dev_get_by_id(HLD_DEV_TYPE_AES, i);
		if ( pAesDev == NULL )
		{
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: pAesDev is NULL\n"); 
			return RET_FAILURE ;
		}
		
		aes_fd = pAesDev->fd;
		dev_free(pAesDev);
	}

	if(aes_fd > 0)
	{
		close(aes_fd);
	}

	ALI_DSC_TDS_HLD_PRF("aes_api_detached\n"); 
	
	return RET_SUCCESS;    
}

int csa_api_attach( void )
{
	pCSA_DEV pCsaDev=NULL;
	UINT32 i=0;
	int csa_fd=-1;
	int ret = -1;
	struct dsc_see_dev_hld see_dev_hld;

	csa_dev_name[strlen(csa_dev_name)-1] = '0';
	
	csa_fd = open(ALI_CSA_LINUX_DEV_PATH, O_RDWR);	
	if(csa_fd < 0)
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: open csa_fd\n"); 
		return csa_fd;
	}
	
	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		csa_dev_name[strlen(csa_dev_name)-1] += (char)1; 
		see_dev_hld.dsc_dev_id = i;
		ret = ioctl(csa_fd, IO_GET_DEV_HLD, (UINT32)&see_dev_hld);
		if(ret < 0)
		{
			close(csa_fd);
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: IO_GET_DEV_HLD\n"); 
			return RET_FAILURE;
		}
		
		pCsaDev = dev_alloc(csa_dev_name, HLD_DEV_TYPE_CSA, sizeof(CSA_DEV));
		if ( pCsaDev == NULL )
		{
			close(csa_fd);
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_alloc\n"); 
			return RET_FAILURE;
		}

		pCsaDev->id_number = i;
		pCsaDev->fd = csa_fd;
		pCsaDev->priv = (void *)(see_dev_hld.dsc_dev_hld);
		if ( dev_register(pCsaDev) != RET_SUCCESS )
		{
			close(csa_fd);
			dev_free(pCsaDev);
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_register\n"); 
			return RET_FAILURE;
		}
		else
		{
			ALI_DSC_TDS_HLD_PRF("CSA attach success, pCsaDev->fd=%d\n",csa_fd);
		}
		
	}
	
	return RET_SUCCESS;
}

static int csa_api_detach(void)
{
	pCSA_DEV pCsaDev=NULL;
	UINT32 i=0;
	int csa_fd = -1;
	
	for(i=0;i<VIRTUAL_DEV_NUM;i++)
	{
		pCsaDev = dev_get_by_id(HLD_DEV_TYPE_CSA, i);
		if ( pCsaDev == NULL )
		{
			ALI_DSC_TDS_HLD_PRF("Dsc hld error: pCsaDev is NULL\n"); 
			return RET_FAILURE;
		}

		csa_fd = pCsaDev->fd;
		dev_free(pCsaDev);
	}
	
	if(csa_fd > 0)
	{
		close(csa_fd);
	}

	ALI_DSC_TDS_HLD_PRF("csa_api_detached\n"); 
	return RET_SUCCESS;    
}

int dsc_api_attach(void)
{
	pDSC_DEV pDscDev = NULL;
	int dsc_fd = -1;

	/*
	pDscDev = dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	if ( pDscDev )
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld warning: pDscDev has already been attached!\n"); 
		return RET_SUCCESS;
	}
	*/
	
	pDscDev = dev_alloc(dsc_dev_name, HLD_DEV_TYPE_DSC, sizeof(DSC_DEV));
	if ( pDscDev == NULL )
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_alloc\n"); 
		return RET_FAILURE;
	}
	if ( dev_register(pDscDev) != RET_SUCCESS )
	{
		dev_free(pDscDev);
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: dev_register\n"); 
		return RET_FAILURE;
	}

	dsc_fd = open(ALI_DSC_LINUX_DEV_PATH, O_RDWR);
	if (dsc_fd < 0)
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: open dev\n"); 
		return RET_FAILURE;
	}	
	pDscDev->fd = dsc_fd;

	pDscDev->user_base =  mmap(NULL, DSC_U_MEM_SIZE, PROT_READ | PROT_WRITE, \
								MAP_SHARED, dsc_fd, 0);
	if(pDscDev->user_base == MAP_FAILED)
	{
		ALI_DSC_TDS_HLD_PRF("dsc mmap error!\n");
		close(dsc_fd);
		return RET_FAILURE;
	}
	else
	{
		ALI_DSC_TDS_HLD_PRF("dsc mmap ok!\n"); 
	}
	
	if(RET_SUCCESS != aes_api_attach())
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: aes_api_attach\n"); 
		close(dsc_fd);
		return RET_FAILURE;
	}
		
	if(RET_SUCCESS != csa_api_attach())
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: csa_api_attach\n"); 
		close(dsc_fd);
		return RET_FAILURE;
	}

	if(RET_SUCCESS != sha_api_attach())
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: sha_api_attach\n"); 
		return RET_FAILURE;
	}

	if(RET_SUCCESS != des_api_attach())
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: des_api_attach\n"); 
		close(dsc_fd);
		return RET_FAILURE;
	}

	ALI_DSC_TDS_HLD_PRF("Dsc attached successfully\n");
	
	return RET_SUCCESS;
}


int dsc_api_detach(void)
{
	pDSC_DEV pDscDev = NULL;

	pDscDev = dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	if ( pDscDev == NULL )
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: pDscDev is NULL\n"); 
		return RET_FAILURE;
	}

	if(RET_SUCCESS != sha_api_detach())
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: sha_api_detach\n"); 
		return RET_FAILURE;
	}

	if(RET_SUCCESS != aes_api_detach())
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: aes_api_detach\n"); 
		return RET_FAILURE;
	}
	
	if(RET_SUCCESS != des_api_detach())
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: des_api_detach\n"); 
		return RET_FAILURE;
	}
	
	if(RET_SUCCESS != csa_api_detach())
	{
		ALI_DSC_TDS_HLD_PRF("Dsc hld error: csa_api_detach\n"); 
		return RET_FAILURE;
	}

	if(pDscDev->fd > 0)
	{
		close(pDscDev->fd);
	}
	
	dev_free(pDscDev);
	ALI_DSC_TDS_HLD_PRF("\nDsc detach done\n");  
	return RET_SUCCESS;
}


