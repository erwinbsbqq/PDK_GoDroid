
#ifndef ALI_MEMPOOL_H
#define ALI_MEMPOOL_H


#define KB 1024
#define MB (1024 * KB)

#include <../../mips/ali/m39/mach/map.h>


struct ALiPools{
    const char *name;
    u32 size;
    unsigned int phy_addr;
};


#define ALI_MAX_MEMPOOLS  1 //[20110506]:FB1

/*
 * A mempool descriptor, used by the driver to track allocations.
 * @list: A Linux linked list head.
 * @name: The name of the pool using this descriptor; useful for printing.
 * @size: The size of this allocation
 * @in_use: Set to true if this memory is in use; if false it is available for
 * allocation.
 * @phys: The physical address of the beginning of the allocation.
 */
struct ali_mempool_desc {
	struct 		list_head list;
	const char	*name;
	u32 		size;
	bool		in_use;
	void		*phys;
	u32		user_virt;
	u32		private_data;
};


#endif /* ALI_MEMPOOL_H */
