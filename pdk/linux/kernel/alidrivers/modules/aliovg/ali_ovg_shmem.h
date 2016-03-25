
#ifndef ALI_SHMEM_H
#define ALI_SHMEM_H


struct shmem_entry {
	unsigned int phy_addr;
	unsigned int size;
	unsigned int key;
	unsigned int ref;
	struct list_head next;
};


#endif /* ALI_MEMPOOL_H */
