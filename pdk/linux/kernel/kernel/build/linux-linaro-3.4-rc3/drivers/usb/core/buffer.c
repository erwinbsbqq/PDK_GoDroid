/*
 * DMA memory management for framework level HCD code (hc_driver)
 *
 * This implementation plugs in through generic "usb_bus" level methods,
 * and should work with all USB controllers, regardles of bus type.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>

extern bool patch_s3921;
/*
 * DMA-Coherent Buffers
 */

/* FIXME tune these based on pool statistics ... */
static const size_t	pool_max[HCD_BUFFER_POOLS] = {
	/* platforms without dma-friendly caches might need to
	 * prevent cacheline sharing...
	 */
	32,
	128,
	512,
	PAGE_SIZE / 2
	/* bigger --> allocate pages */
};


/* SETUP primitives */

/**
 * hcd_buffer_create - initialize buffer pools
 * @hcd: the bus whose buffer pools are to be initialized
 * Context: !in_interrupt()
 *
 * Call this as part of initializing a host controller that uses the dma
 * memory allocators.  It initializes some pools of dma-coherent memory that
 * will be shared by all drivers using that controller, or returns a negative
 * errno value on error.
 *
 * Call hcd_buffer_destroy() to clean up after using those pools.
 */
int hcd_buffer_create(struct usb_hcd *hcd)
{
	char		name[16];
	int		i, size;

	if (!hcd->self.controller->dma_mask &&
	    !(hcd->driver->flags & HCD_LOCAL_MEM))
		return 0;

	for (i = 0; i < HCD_BUFFER_POOLS; i++) {
		size = pool_max[i];
		if (!size)
			continue;
		snprintf(name, sizeof name, "buffer-%d", size);
		hcd->pool[i] = dma_pool_create(name, hcd->self.controller,
				size, size, 0);
		if (!hcd->pool[i]) {
			hcd_buffer_destroy(hcd);
			return -ENOMEM;
		}
	}
	return 0;
}


/**
 * hcd_buffer_destroy - deallocate buffer pools
 * @hcd: the bus whose buffer pools are to be destroyed
 * Context: !in_interrupt()
 *
 * This frees the buffer pools created by hcd_buffer_create().
 */
void hcd_buffer_destroy(struct usb_hcd *hcd)
{
	int i;

	for (i = 0; i < HCD_BUFFER_POOLS; i++) {
		struct dma_pool *pool = hcd->pool[i];
		if (pool) {
			dma_pool_destroy(pool);
			hcd->pool[i] = NULL;
		}
	}
}


/* sometimes alloc/free could use kmalloc with GFP_DMA, for
 * better sharing and to leverage mm/slab.c intelligence.
 */

void *hcd_buffer_alloc(
	struct usb_bus		*bus,
	size_t			size,
	gfp_t			mem_flags,
	dma_addr_t		*dma
)
{
	struct usb_hcd		*hcd = bus_to_hcd(bus);
	int			i;

       //#ifdef CONFIG_PATCH_ALI_S3921_EHCI
        void *tmp;
       //#endif

	/* some USB hosts just use PIO */
	if (!bus->controller->dma_mask &&
	    !(hcd->driver->flags & HCD_LOCAL_MEM)) {
		*dma = ~(dma_addr_t) 0;
		return kmalloc(size, mem_flags);
	}

	for (i = 0; i < HCD_BUFFER_POOLS; i++) {
		if (size <= pool_max[i]){	
                //#ifdef CONFIG_PATCH_ALI_S3921_EHCI
                if (patch_s3921)
                {	
                   tmp = dma_pool_alloc(hcd->pool[i], mem_flags, dma);
                   *dma &= 0x7fffffff;
                   return tmp;
                }                
                //#else
                else
               {	 
	        	return dma_pool_alloc(hcd->pool[i], mem_flags, dma);
        	}		
              //#endif
            }
	}
	//#ifdef CONFIG_PATCH_ALI_S3921_EHCI
	if (patch_s3921)
	{	
            tmp = dma_alloc_coherent(hcd->self.controller, size, dma, mem_flags);
            *dma &= 0x7fffffff;
            return tmp;
        }  
	 //#else
	 else
	 {	
	    return dma_alloc_coherent(hcd->self.controller, size, dma, mem_flags);
	 }		
	 //#endif        
}

void hcd_buffer_free(
	struct usb_bus		*bus,
	size_t			size,
	void			*addr,
	dma_addr_t		dma
)
{
	struct usb_hcd		*hcd = bus_to_hcd(bus);
	int			i;

	if (!addr)
		return;

	if (!bus->controller->dma_mask &&
	    !(hcd->driver->flags & HCD_LOCAL_MEM)) {
		kfree(addr);
		return;
	}

	for (i = 0; i < HCD_BUFFER_POOLS; i++) {
		if (size <= pool_max[i]) {
			//#ifdef CONFIG_PATCH_ALI_S3921_EHCI
			if (patch_s3921)
			{	
				dma |= 0x80000000; 
			}	
			//#endif
			dma_pool_free(hcd->pool[i], addr, dma);
			return;
		}
	}

  if (patch_s3921)
  {	 
  	//#ifdef CONFIG_PATCH_ALI_S3921_EHCI
		dma |= 0x80000000; 
    //#endif
  } 
	dma_free_coherent(hcd->self.controller, size, addr, dma);
}
