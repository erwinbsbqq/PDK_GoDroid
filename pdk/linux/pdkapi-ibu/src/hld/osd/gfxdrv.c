#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <api/libc/alloc.h>
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>
#include <hld/ge/ge.h>
#include <hld/osd/gfxdrv.h>
#define PRINTF(...)  do{}while(0)
#define SURFACE_PRINT_ERROR  PRINTF
#define SURFACE_PRINT  PRINTF

static int    initial_flag = 0;
static int    fbfd;
static struct fb_fix_screeninfo fix;
static void * pGe;
static struct ge_device * ge_dev = NULL;

error_code gfxdrv_init(void)
{

  if((fbfd = open("/dev/fb0", O_RDWR)) < 0) {
    SURFACE_PRINT_ERROR("Err: can not open /dev/fb0\n");
    return -ERR_SYS;
  }

  if(ioctl(fbfd, FBIOGET_FSCREENINFO, &fix)) {
    close(fbfd);
    SURFACE_PRINT_ERROR("Err: fixscreeninfo ioctl\n");
    return -ERR_SYS;
  }

  if ((pGe = mmap(MAP_GE_BASE_ADDRESS_START, fix.mmio_len, PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, fix.smem_len)) == (void *)-1){
    close(fbfd);
    SURFACE_PRINT_ERROR("Err: mmap ge failed\n");
    return -ERR_SYS;
  }

  ge_m36f_attach(NULL, 0);
  ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
  if(ge_dev == NULL)
  {
    SURFACE_PRINT_ERROR("Err: get ge device failed\n");
    //TBD: detach ge m36f
    munmap(pGe, fix.mmio_len);
    close(fbfd);
    return -ERR_SYS;
  }
  ge_open(ge_dev);
  initial_flag = 1;

  return 0;
}

error_code gfxdrv_exit(void)
{
  if(initial_flag)
  {
    //TBD: to close ge_dev and detach 
    ge_dev = NULL;
    munmap(pGe, fix.mmio_len);
    close(fbfd);
    initial_flag = 0;
  }
  return 0;
}
//Fill a region of an ARGB8888 surface with given color value
error_code gfxdrv_color_fill(const gfxdrv_surface *dest, const gfxdrv_rect *dest_rect, unsigned int argb_color)
{
  unsigned int cmd_hdl;
  ge_base_addr_t base_addr_dst; 
 // ge_cmd_list_hdl cmd_list = GE_IO_REG;
  
  if(dest == NULL || dest_rect == NULL)
  {
    return -ERR_INVALIED_PARA;
  }
  
  if(ge_dev == NULL)
  {
    return -ERR_INVALIED_PARA;
  } 
  SURFACE_PRINT("base:0x%08lx,format:%d,width:%d,height:%d,rgb:0x%08lx\n",\
                  dest->physical_address,dest->format,dest->width,dest->height,argb_color);
  
  if(dest_rect->x < 0
     ||dest_rect->y < 0 
     ||dest_rect->width < 0 
     ||dest_rect->width > 1280
     ||dest_rect->height < 0
     ||dest_rect->height > 720)
  {
     return -1;
  }
  ge_cmd_list_hdl cmd_list = ge_cmd_list_create(ge_dev, 1);
  ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
  
  base_addr_dst.base_address =(unsigned int)dest->physical_address;
  if (dest->format == surface_format_MONO)
  {
  	base_addr_dst.color_format = GE_PF_CLUT8;
  	base_addr_dst.pixel_pitch = dest->pitch;
  }
  else
  {
	base_addr_dst.color_format = GE_PF_ARGB8888;  	
  	base_addr_dst.pixel_pitch = dest->pitch/4;
  }	
  base_addr_dst.data_decoder = GE_DECODER_DISABLE;    
  base_addr_dst.modify_flags = GE_BA_FLAG_ALL; 
  
  base_addr_dst.base_addr_sel = GE_BASE_ADDR;    
  ge_set_base_addr(ge_dev, cmd_list, GE_DST, &base_addr_dst);

  //Test primitive mode: fill rect back color
  cmd_hdl = ge_cmd_begin(ge_dev, cmd_list, GE_FILL_RECT_BACK_COLOR);
  ge_set_back_color(ge_dev, cmd_hdl, argb_color);
  ge_set_xy(ge_dev, cmd_hdl, GE_DST, dest_rect->x, dest_rect->y);
  ge_set_wh(ge_dev, cmd_hdl, GE_DST, dest_rect->width, dest_rect->height); 
  ge_cmd_end(ge_dev, cmd_hdl);
    
  ge_cmd_list_end(ge_dev, cmd_list);
  ge_cmd_list_start(ge_dev, cmd_list, TRUE, TRUE);
  ge_cmd_list_destroy(ge_dev, cmd_list);

  return 0;
}

error_code gfxdrv_color_blend(const gfxdrv_surface *dest,const gfxdrv_rect *dest_rect,unsigned int argb_color)
{
  unsigned int cmd_hdl;
  ge_base_addr_t base_addr_dst; 
  //ge_cmd_list_hdl cmd_list = GE_IO_REG;
  
  if(dest == NULL || dest_rect == NULL)
  {
    return -ERR_INVALIED_PARA;
  }
  
  if(ge_dev == NULL)
  {
    return -ERR_INVALIED_PARA;
  } 
  SURFACE_PRINT("base:0x%08lx,format:%d,width:%d,height:%d,rgb:0x%08lx\n",\
                  dest->physical_address,dest->format,dest->width,dest->height,argb_color);
  
  if(dest_rect->x < 0
     || dest_rect->y < 0 
     ||dest_rect->width < 0 
     ||dest_rect->width > 1280
     ||dest_rect->height < 0
     ||dest_rect->height > 720)
  {
     return -1;
  }
  ge_cmd_list_hdl cmd_list = ge_cmd_list_create(ge_dev, 1);
  ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
  
  base_addr_dst.base_address = (unsigned int)dest->physical_address;
  if (dest->format == surface_format_MONO)
  {
  	base_addr_dst.color_format = GE_PF_CLUT8;
  	base_addr_dst.pixel_pitch = dest->pitch;
  }
  else
  {
	base_addr_dst.color_format = GE_PF_ARGB8888;  	
 	base_addr_dst.pixel_pitch = dest->pitch/4;
  }	
  base_addr_dst.data_decoder = GE_DECODER_DISABLE;    
  base_addr_dst.modify_flags = GE_BA_FLAG_ALL; 
  
  base_addr_dst.base_addr_sel = GE_BASE_ADDR;    
  ge_set_base_addr(ge_dev, cmd_list, GE_DST, &base_addr_dst);    
  ge_set_base_addr(ge_dev, cmd_list, GE_SRC, &base_addr_dst);

  cmd_hdl = ge_cmd_begin(ge_dev, cmd_list, GE_FILL_RECT_BACK_COLOR);
  ge_set_rop_mode(ge_dev, cmd_hdl, GE_ROP_ALPHA_BLENDING);
  ge_set_back_color(ge_dev, cmd_hdl, argb_color);
  ge_set_xy(ge_dev, cmd_hdl, GE_DST_SRC, dest_rect->x, dest_rect->y);
  ge_set_wh(ge_dev, cmd_hdl, GE_DST_SRC, dest_rect->width, dest_rect->height);  
  ge_cmd_end(ge_dev, cmd_hdl);
    
  ge_cmd_list_end(ge_dev, cmd_list);
  ge_cmd_list_start(ge_dev, cmd_list, TRUE, TRUE);
  ge_cmd_list_destroy(ge_dev, cmd_list);
  return 0;
}

//Copy a region of any surface to an ARGB8888 surfac
error_code gfxdrv_copy(const gfxdrv_surface *dest,unsigned int dest_x, unsigned int dest_y, \
                               const gfxdrv_surface *src,const gfxdrv_rect *src_rect, \
                               unsigned int src_color)
{

  unsigned int cmd_hdl;
  ge_base_addr_t base_addr_dst,base_addr_src; 
//  ge_cmd_list_hdl cmd_list = GE_IO_REG;
  
  if(dest == NULL || src == NULL || src_rect == NULL)
  {
    return -ERR_INVALIED_PARA;
  }
  
  if(ge_dev == NULL)
  {
    return -ERR_INVALIED_PARA;
  } 
  SURFACE_PRINT("dest adderss:0x%08lx,format:%d,src address:0x%08lx,format:%d,rgb:%d\n",\
                  dest->physical_address,dest->format,src->physical_address,src->format,src_color);
  
  if(src_rect->x < 0 ||dest_x < 0
     ||src_rect->y < 0 ||dest_y < 0 
     ||src_rect->width < 0 
     ||src_rect->width > 1280
     ||src_rect->height < 0
     ||src_rect->height > 720)
  {
     return -1;
  }
  ge_cmd_list_hdl cmd_list = ge_cmd_list_create(ge_dev, 1);
  ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
  
  base_addr_dst.base_address = (unsigned int)dest->physical_address;
  if (dest->format == surface_format_MONO)
  {	  
  	base_addr_dst.color_format = GE_PF_CLUT8;
  	base_addr_dst.pixel_pitch = dest->pitch;
  }
  else
  {
	base_addr_dst.color_format = GE_PF_ARGB8888;  	
  	base_addr_dst.pixel_pitch = dest->pitch/4;
  }	
  base_addr_dst.data_decoder = GE_DECODER_DISABLE;    
  base_addr_dst.modify_flags = GE_BA_FLAG_ALL; 
  base_addr_dst.base_addr_sel = GE_BASE_ADDR;    
  ge_set_base_addr(ge_dev, cmd_list, GE_DST, &base_addr_dst);
  
  base_addr_src.base_address = (unsigned int)src->physical_address;
  if (src->format == surface_format_MONO)
  {
  	base_addr_src.color_format = GE_PF_CLUT8;
  	base_addr_src.pixel_pitch = src->pitch;
  }
  else
  {
	base_addr_src.color_format = GE_PF_ARGB8888;  	
  	base_addr_src.pixel_pitch = src->pitch/4;
  }
  base_addr_src.data_decoder = GE_DECODER_DISABLE;    
  base_addr_src.modify_flags = GE_BA_FLAG_ALL; 
  base_addr_src.base_addr_sel = GE_BASE_ADDR1;    
  ge_set_base_addr(ge_dev, cmd_list, GE_SRC, &base_addr_src);    

  cmd_hdl = ge_cmd_begin(ge_dev, cmd_list, GE_PRIM_DISABLE);
  ge_set_src_mode(ge_dev, cmd_hdl, GE_SRC_BITBLT);
  if(src->format == surface_format_MONO)
  	ge_set_back_color(ge_dev, cmd_hdl, src_color);
  ge_set_xy(ge_dev, cmd_hdl, GE_DST, dest_x, dest_y);
  ge_set_xy(ge_dev, cmd_hdl, GE_SRC, src_rect->x, src_rect->y);
  ge_set_wh(ge_dev, cmd_hdl, GE_DST_SRC, src_rect->width, src_rect->height); 
  ge_cmd_end(ge_dev, cmd_hdl);
    
  ge_cmd_list_end(ge_dev, cmd_list);
  ge_cmd_list_start(ge_dev, cmd_list, TRUE, TRUE);
  ge_cmd_list_destroy(ge_dev, cmd_list);
  return 0;
}
// Direct Copy a region of any surface to an ARGB8888 surface
// source surface format must be same as dest format
error_code gfxdrv_direct_copy(const gfxdrv_surface *dest,unsigned int dest_x, unsigned int dest_y, \
                               const gfxdrv_surface *src,const gfxdrv_rect *src_rect)
{

  unsigned int cmd_hdl;
  ge_base_addr_t base_addr_dst,base_addr_src; 
  //ge_cmd_list_hdl cmd_list = GE_IO_REG;
  
  if(dest == NULL || src == NULL || src_rect == NULL)
  {
    return -ERR_INVALIED_PARA;
  }
  
  if(ge_dev == NULL)
  {
    return -ERR_INVALIED_PARA;
  } 
  SURFACE_PRINT("dest adderss:0x%08lx,format:%d,src address:0x%08lx,format:%d,rgb:%d\n",\
                  dest->physical_address,dest->format,src->physical_address,src->format,src_color);
  
  if(src_rect->x < 0 ||dest_x < 0
     ||src_rect->y < 0 ||dest_y < 0 
     ||src_rect->width < 0 
     ||src_rect->width > 1280
     ||src_rect->height < 0
     ||src_rect->height > 720)
  {
     return -1;
  }

  if (dest->format != src->format)
  {
     return -1;
  }
  
  ge_cmd_list_hdl cmd_list = ge_cmd_list_create(ge_dev, 1);
  ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
  
  base_addr_dst.base_address = (unsigned int)dest->physical_address;
  if (dest->format == surface_format_MONO)
  {	  
  	base_addr_dst.color_format = GE_PF_CLUT8;
  	base_addr_dst.pixel_pitch = dest->pitch;
  }
  else
  {
	base_addr_dst.color_format = GE_PF_ARGB8888;  	
  	base_addr_dst.pixel_pitch = dest->pitch/4;
  }	
  base_addr_dst.data_decoder = GE_DECODER_DISABLE;    
  base_addr_dst.modify_flags = GE_BA_FLAG_ALL; 
  base_addr_dst.base_addr_sel = GE_BASE_ADDR;    
  ge_set_base_addr(ge_dev, cmd_list, GE_DST, &base_addr_dst);
  
  base_addr_src.base_address = (unsigned int)src->physical_address;
  if (src->format == surface_format_MONO)
  {
  	base_addr_src.color_format = GE_PF_CLUT8;
  	base_addr_src.pixel_pitch = src->pitch;
  }
  else
  {
	base_addr_src.color_format = GE_PF_ARGB8888;  	
  	base_addr_src.pixel_pitch = src->pitch/4;
  }
  base_addr_src.data_decoder = GE_DECODER_DISABLE;    
  base_addr_src.modify_flags = GE_BA_FLAG_ALL; 
  base_addr_src.base_addr_sel = GE_BASE_ADDR1;    
  ge_set_base_addr(ge_dev, cmd_list, GE_SRC, &base_addr_src);    

  cmd_hdl = ge_cmd_begin(ge_dev, cmd_list, GE_PRIM_DISABLE);
  ge_set_src_mode(ge_dev, cmd_hdl, GE_SRC_DIRECT_COPY);
  ge_set_xy(ge_dev, cmd_hdl, GE_DST, dest_x, dest_y);
  ge_set_xy(ge_dev, cmd_hdl, GE_SRC, src_rect->x, src_rect->y);
  ge_set_wh(ge_dev, cmd_hdl, GE_DST_SRC, src_rect->width, src_rect->height); 
  ge_cmd_end(ge_dev, cmd_hdl);
    
  ge_cmd_list_end(ge_dev, cmd_list);
  ge_cmd_list_start(ge_dev, cmd_list, TRUE, TRUE);
  ge_cmd_list_destroy(ge_dev, cmd_list);
  return 0;
}
                               
//Blend a region of any surface to an ARGB8888 surface, with scaling on-the-fl
error_code gfxdrv_scale_lld(const gfxdrv_surface *dest,const gfxdrv_rect *dest_rect,const gfxdrv_surface *src,const gfxdrv_rect *src_rect)
{

  unsigned int cmd_hdl;
  gfxdrv_surface surf;
  ge_scale_info_t pscale_info;
  ge_base_addr_t base_addr_dst,base_addr_src; 
  ge_cmd_list_hdl cmd_list = GE_IO_REG;
  
  if(dest == NULL || dest_rect == NULL ||src == NULL || src_rect == NULL )
  {
    return -ERR_INVALIED_PARA;
  }
  
  if(ge_dev == NULL)
  {
    return -ERR_INVALIED_PARA;
  } 
  SURFACE_PRINT("dest adderss:0x%08lx,format:%d,src address:0x%08lx,format:%d\n",\
                  dest->physical_address,dest->format,src->physical_address,src->format);
  
  ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
  
  base_addr_dst.base_address =(unsigned int)dest->physical_address;
  if (dest->format == surface_format_MONO)
  {
  	base_addr_dst.color_format = GE_PF_CLUT8;
  	base_addr_dst.pixel_pitch = dest->pitch;
  }
  else
  {
	base_addr_dst.color_format = GE_PF_ARGB8888;  	
  	base_addr_dst.pixel_pitch = dest->pitch/4;
  }	
  base_addr_dst.data_decoder = GE_DECODER_DISABLE;    
  base_addr_dst.modify_flags = GE_BA_FLAG_ALL; 
  
  base_addr_src.base_addr_sel = GE_BASE_ADDR;    
  
  base_addr_src.base_address = (unsigned int)src->physical_address;
  if (src->format == surface_format_MONO)
  {
  	base_addr_src.color_format = GE_PF_CLUT8;
  	base_addr_src.pixel_pitch = dest->pitch;
  }	
  else
  {
	  base_addr_src.color_format = GE_PF_ARGB8888;  	
  	base_addr_src.pixel_pitch = dest->pitch/4;
  }	
  base_addr_src.data_decoder = GE_DECODER_DISABLE;    
  base_addr_src.modify_flags = GE_BA_FLAG_ALL; 
  base_addr_src.base_addr_sel = GE_BASE_ADDR1;    
       
  pscale_info.blend_info_en = FALSE;
  pscale_info.blend_info_addr = 0;
  pscale_info.color_premuled = 1;

  cmd_hdl = ge_cmd_begin(ge_dev, cmd_list, GE_PRIM_SCALING);
  ge_set_scale_info(ge_dev, cmd_hdl,&pscale_info);
  ge_set_base_addr(ge_dev, cmd_list, GE_SRC, &base_addr_src);
  ge_set_base_addr(ge_dev, cmd_list, GE_DST, &base_addr_dst);
  ge_set_xy(ge_dev, cmd_hdl, GE_DST, dest_rect->x, dest_rect->y);
  ge_set_wh(ge_dev, cmd_hdl, GE_DST, dest_rect->width, dest_rect->height);
  ge_set_xy(ge_dev, cmd_hdl, GE_SRC, src_rect->x, src_rect->y);
  ge_set_wh(ge_dev, cmd_hdl, GE_SRC, src_rect->width, src_rect->height); 
  ge_cmd_end(ge_dev, cmd_hdl);
    
  ge_cmd_list_end(ge_dev, cmd_list);
  ge_cmd_list_start(ge_dev, cmd_list, TRUE, TRUE);
  ge_cmd_list_destroy(ge_dev, cmd_list);
  return 0;
}

//Blend a region of any surface to an ARGB8888 surface, with scaling on-the-fl
error_code gfxdrv_scale(const gfxdrv_surface *dest,const gfxdrv_rect *dest_rect,const gfxdrv_surface *src,const gfxdrv_rect *src_rect)
{
  int ret;
  unsigned int cmd_hdl;
  gfxdrv_surface src_copy_surf,dest_copy_surf;
  gfxdrv_rect src_copy_rect,dest_copy_rect;  
    
  if(dest == NULL || dest_rect == NULL ||src == NULL || src_rect == NULL )
  {
    return -ERR_INVALIED_PARA;
  }
  
  if(ge_dev == NULL)
  {
    return -ERR_INVALIED_PARA;
  } 
  printf("dest adderss:0x%08lx,format:%d,src address:0x%08lx,format:%d\n",\
                  dest->physical_address,dest->format,src->physical_address,src->format);
  /* patch for bytes align 
   1.src_rect->width > 8
   2.src_rect->x > 0;src_rect->y >0; 0<src_rect->w<1920;0<src_rect->h<1080;
   3.dest_rect->x > 0;dest_rect->y >0; 0<dest_rect->w<1920;0<dest_rect->h<1080;
   4.src->pitch&0x0f ==0,dest->pitch&0x0f ==0; 16pixel align
   5.(src->addr+src->y*src->pitch*4+src->x*4)&0x3f ==0  64byte align 
  */

  if(src_rect->x < 0 ||dest_rect->x < 0
     ||src_rect->y < 0 ||dest_rect->y < 0 
     ||dest_rect->width < 0 
     ||dest_rect->width > 1280
     ||dest_rect->height < 0
     ||dest_rect->height > 720
     ||src_rect->width < 0 
     ||src_rect->width > 1280
     ||src_rect->height < 0
     ||src_rect->height > 720)
  {
     return -ERR_INVALIED_PARA;
  }
 
 if ((src->format == surface_format_ARGB8888)&&
      (((((unsigned int)src->physical_address+src_rect->y*src->pitch+src_rect->x*4)&0x3f) !=0)      
      ||((((unsigned int)dest->physical_address+dest_rect->y*dest->pitch+dest_rect->x*4)&0x3f) !=0)))
  {
        src_copy_surf.width = src->width;
    	src_copy_surf.height = src->height;
      	src_copy_surf.pitch  = src->pitch;
 	    src_copy_surf.format = src->format;
      	src_copy_surf.physical_address = 0x8000000;
        printf("dest-width=%d\n", dest_rect->width);
	    src_copy_rect.x = 0;
    	src_copy_rect.y = 0;
       	src_copy_rect.width = src_rect->width;
   	    src_copy_rect.height = src_rect->height;
                
	    dest_copy_surf.width = dest->width;
      	dest_copy_surf.height = dest->height;
       	dest_copy_surf.pitch  = dest->pitch;
    	dest_copy_surf.format = dest->format;
    	dest_copy_surf.physical_address = 0x8000000+0x384000;
		
	dest_copy_rect.x = 0;
      	dest_copy_rect.y = 0;
     	dest_copy_rect.width = dest_rect->width;
    	dest_copy_rect.height = dest_rect->height;

        ret=gfxdrv_copy(&src_copy_surf,0,0,src,src_rect,0);
	if(ret != 0)
      	{
  		SURFACE_PRINT("gfxdrv_scale_copy erro\n");
        	return -1; 
      	}
    
  	ret=gfxdrv_scale_lld(&dest_copy_surf,&dest_copy_rect,&src_copy_surf,&src_copy_rect);
  	if(ret != 0)
  	{
  		SURFACE_PRINT("gfxdrv_scale erro\n");
        	return -1; 
  	}
  	ret=gfxdrv_copy(dest,dest_rect->x,dest_rect->y,&dest_copy_surf,&dest_copy_rect,0);
  	if(ret != 0)
  	{
  		SURFACE_PRINT("gfxdrv_scale_copy erro\n");
        	return -1; 
  	} 
        return 0;
  }
  if ((src->format == surface_format_MONO)&&
      (((((unsigned int)src->physical_address+src_rect->y*src->pitch+src_rect->x)&0x3f) !=0)      
      ||((((unsigned int)dest->physical_address+dest_rect->y*dest->pitch+dest_rect->x)&0x3f) !=0)))
  {
        src_copy_surf.width = src->width;
    	src_copy_surf.height = src->height;
      	src_copy_surf.pitch  = src->pitch;
 	    src_copy_surf.format = src->format;
      	src_copy_surf.physical_address = 0x8000000;

	    src_copy_rect.x = 0;
    	src_copy_rect.y = 0;
       	src_copy_rect.width = src_rect->width;
   	    src_copy_rect.height = src_rect->height;
                
	    dest_copy_surf.width = dest->width;
      	dest_copy_surf.height = dest->height;
       	dest_copy_surf.pitch  = dest->pitch;
    	dest_copy_surf.format = dest->format;
    	dest_copy_surf.physical_address = 0x8000000+0x384000;
		
	dest_copy_rect.x = 0;
      	dest_copy_rect.y = 0;
     	dest_copy_rect.width = dest_rect->width;
    	dest_copy_rect.height = dest_rect->height;
 
  	ret=gfxdrv_copy(&src_copy_surf,0,0,src,src_rect,0);
      	if(ret != 0)
      	{
  		SURFACE_PRINT("gfxdrv_scale_copy erro\n");
        	return -1; 
      	}
    
      	ret=gfxdrv_scale_lld(&dest_copy_surf,&dest_copy_rect,&src_copy_surf,&src_copy_rect);
  	if(ret != 0)
  	{
  		SURFACE_PRINT("gfxdrv_scale erro\n");
        	return -1; 
  	}
        
  	ret=gfxdrv_copy(dest,dest_rect->x,dest_rect->y,&dest_copy_surf,&dest_copy_rect,0);
  	if(ret != 0)
  	{
  		SURFACE_PRINT("gfxdrv_scale_copy erro\n");
        	return -1; 
      	}
        return 0;
  }
  ret=gfxdrv_scale_lld(dest,dest_rect,src,src_rect);
  if(ret != 0)
  {
  	SURFACE_PRINT("gfxdrv_scale erro\n");
        return -1; 
  }
  return 0;
}

error_code gfxdrv_blend(const gfxdrv_surface *dest,unsigned int dest_x, unsigned int dest_y, \
                         const gfxdrv_surface *src,const gfxdrv_rect *src_rect, \
                         unsigned int src_color,unsigned char src_alpha)
{

  unsigned int cmd_hdl;
  ge_base_addr_t base_addr_dst,base_addr_src; 
  //ge_cmd_list_hdl cmd_list = GE_IO_REG;
  
  if(dest == NULL || src == NULL || src_rect == NULL)
  {
    return -ERR_INVALIED_PARA;
  }
  
  if(ge_dev == NULL)
  {
    return -ERR_INVALIED_PARA;
  } 
  SURFACE_PRINT("dest base:0x%08lx,format:%d,src base:0x%08lx,src format:%d,rgb:%d\n",\
                  dest->physical_address,dest->format,src->physical_address,src->format,src_color);
  
  if(dest_x < 0
     ||dest_y < 0 
     ||src_rect->width < 0 
     ||src_rect->width > 1280
     ||src_rect->height < 0
     ||src_rect->height > 720)
  {
     return -1;
  }
  ge_cmd_list_hdl cmd_list = ge_cmd_list_create(ge_dev, 1);
  ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
  
  base_addr_dst.base_address = (unsigned int)dest->physical_address;
  if (dest->format == surface_format_MONO)
  {
  	base_addr_dst.color_format = GE_PF_CLUT8;
  	base_addr_dst.pixel_pitch = dest->pitch;
  }	
  else
  {
	base_addr_dst.color_format = GE_PF_ARGB8888;  	
  	base_addr_dst.pixel_pitch = dest->pitch/4;
  }	
  base_addr_dst.data_decoder = GE_DECODER_DISABLE;        
  base_addr_dst.modify_flags = GE_BA_FLAG_ALL; 
  
  base_addr_dst.base_addr_sel = GE_BASE_ADDR;    

  if (src->format == surface_format_MONO)
  {	  
  	cmd_hdl = ge_cmd_begin(ge_dev, cmd_list, GE_FILL_RECT_BACK_COLOR);

    ge_set_base_addr(ge_dev, cmd_hdl, GE_DST, &base_addr_dst);
    ge_set_base_addr(ge_dev, cmd_hdl, GE_SRC, &base_addr_dst);

  	base_addr_src.color_format = GE_PF_MASK_A8;
  	base_addr_src.pixel_pitch = src->pitch;
    base_addr_src.base_address = (unsigned int)src->physical_address;
  	base_addr_src.data_decoder = GE_DECODER_DISABLE;    
  	base_addr_src.modify_flags = GE_BA_FLAG_ALL;     
 
	base_addr_src.base_addr_sel = GE_BASE_ADDR1;    
	ge_set_base_addr(ge_dev, cmd_hdl, GE_MSK, &base_addr_src); 
	ge_set_global_alpha(ge_dev, cmd_hdl, src_alpha);    
  	ge_set_global_alpha_sel(ge_dev, cmd_hdl, GE_USE_GALPHA_MULTIPLY_PTN_ALPHA);

	ge_set_msk_mode(ge_dev, cmd_hdl, GE_MSK_ENABLE);
	ge_set_rop_mode(ge_dev, cmd_hdl, GE_ROP_ALPHA_BLENDING);
	ge_set_alpha_blend_mode(ge_dev, cmd_hdl, GE_ALPHA_BLEND_SRC_OVER);
	ge_set_alpha_out_mode(ge_dev, cmd_hdl, GE_OUTALPHA_FROM_BLENDING);
    ge_set_back_color(ge_dev, cmd_hdl, src_color);
    ge_set_xy(ge_dev, cmd_hdl, GE_DST_SRC, dest_x, dest_y);
	ge_set_xy(ge_dev, cmd_hdl, GE_MSK, src_rect->x, src_rect->y);
    ge_set_wh(ge_dev, cmd_hdl, GE_MSK, src_rect->width, src_rect->height); 
    ge_set_wh(ge_dev, cmd_hdl, GE_DST_SRC, src_rect->width, src_rect->height); 	 
  	ge_cmd_end(ge_dev, cmd_hdl);
    
  	ge_cmd_list_end(ge_dev, cmd_list);
  	ge_cmd_list_start(ge_dev, cmd_list, TRUE, TRUE);
  	ge_cmd_list_destroy(ge_dev, cmd_list);
  }
  else
  {
    ge_set_base_addr(ge_dev, cmd_list, GE_DST, &base_addr_dst);
    ge_set_base_addr(ge_dev, cmd_list, GE_SRC, &base_addr_dst);
	base_addr_src.color_format = GE_PF_ARGB8888;  	
  	base_addr_src.pixel_pitch = src->pitch/4;
   	base_addr_src.base_address = (unsigned int)src->physical_address;
  	base_addr_src.data_decoder = GE_DECODER_DISABLE;    
  	base_addr_src.modify_flags = GE_BA_FLAG_ALL;     
 
  	base_addr_src.base_addr_sel = GE_BASE_ADDR1;    
  	ge_set_base_addr(ge_dev, cmd_list, GE_PTN, &base_addr_src);
	//ge_set_base_addr(ge_dev, cmd_list, GE_SRC, &base_addr_src);
	    
  	ge_set_global_alpha(ge_dev, cmd_list, src_alpha);    
  	ge_set_global_alpha_sel(ge_dev, cmd_list, GE_USE_GALPHA_MULTIPLY_PTN_ALPHA);
    
  	cmd_hdl = ge_cmd_begin(ge_dev, cmd_list, GE_DRAW_BITMAP_ALPHA_BLENDING);
    	ge_set_xy(ge_dev, cmd_hdl, GE_DST, dest_x, dest_y);
	ge_set_xy(ge_dev, cmd_hdl, GE_PTN, src_rect->x, src_rect->y);
	ge_set_wh(ge_dev, cmd_hdl, GE_DST_PTN, src_rect->width, src_rect->height); 
  	ge_cmd_end(ge_dev, cmd_hdl);
    
  	ge_cmd_list_end(ge_dev, cmd_list);
  	ge_cmd_list_start(ge_dev, cmd_list, TRUE, TRUE);
  	ge_cmd_list_destroy(ge_dev, cmd_list);
  }
  return 0;
}

