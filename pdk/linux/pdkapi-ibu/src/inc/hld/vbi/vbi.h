#ifndef	__HLD_VBI_H__
#define __HLD_VBI_H__


#include <hld/vbi/vbi_dev.h>
#include <rpc_hld/ali_rpc_hld_vbi.h>    //corei

#if 0
#ifdef __cplusplus
extern "C"
{
#endif

INT32 vbi_open(struct vbi_device *dev);
INT32 vbi_close(struct vbi_device *dev);
INT32 vbi_ioctl(struct vbi_device *dev, UINT32 cmd, UINT32 param);
INT32 vbi_request_write(void * pdev,UINT32 uSizeRequested,void ** ppuData,UINT32* puSizeGot,struct control_block* pDataCtrlBlk);
void vbi_update_write(void * pdev,UINT32 uDataSize);
void vbi_setoutput(struct vbi_device *dev,T_VBIRequest *pVBIRequest);

INT32 vbi_start(struct vbi_device *dev,t_TTXDecCBFunc pCBFunc);
INT32 vbi_stop(struct vbi_device *dev);
RET_CODE vbi_io_control(struct vbi_device *dev, UINT32 cmd, UINT32 param);

INT32 ttx_request_page(struct vbi_device *dev, UINT16 page_id , struct PBF_CB ** cb );
INT32 ttx_request_page_up(struct vbi_device *dev,UINT16 page_id , struct PBF_CB ** cb );
INT32 ttx_request_page_down(struct vbi_device *dev, UINT16 page_id , struct PBF_CB ** cb );
void ttx_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set);
void vbi_m36_attach(void);

#ifdef __cplusplus
}
#endif
#endif

#endif /*__HLD_VBI_H__*/






