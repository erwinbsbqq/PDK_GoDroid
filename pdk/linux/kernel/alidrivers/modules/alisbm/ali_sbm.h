#ifndef __ALI_SBM_H_
#define __ALI_SBM_H_

#if 0
#define SBM_PRF(arg, value...)  \
            {\
                printk("<0>""kernel debug : file : %s function : %s line %d\n", __FILE__, __FUNCTION__, __LINE__);\
                printk("<0>"arg, ##value);\
                printk("<0>""kernel debug done\n\n");\
            }
#else
#define SBM_PRF(...)    do{}while(0)
#endif


#if 0
#define PRF(arg, value...)  \
            {\
                printk("kernel debug : file : %s function : %s line %d\n", __FILE__, __FUNCTION__, __LINE__);\
                printk(arg, ##value);\
                printk("kernel debug done\n\n");\
            }
#else
#define PRF(...)    do{}while(0)
#endif

#define SBM_MUTEX_CREATE    ali_rpc_mutex_create
#define SBM_MUTEX_DELETE    ali_rpc_mutex_delete

#define CACHE_ADDR(addr)    (__CACHE_ADDR_ALI(addr))
#define NONCACHE_ADDR(addr) (__NONCACHE_ADDR_ALI(addr))

#if defined(CONFIG_ALI_CHIP_M3921)
static void *m_info_addr[SBM_NUM];

#define SHM_MALLOC(idx, cfg, size)  ((cfg == 0) ? (void *)(m_info_addr[idx]) \
                                        : (void *)(m_info_addr[idx] + sizeof(struct sbm_desc_pkt)))
#define SHM_FREE(ptr)               do{}while(0)

#else

#define SHM_MALLOC(idx, cfg, size)  kmalloc(size, GFP_KERNEL)
#define SHM_FREE(ptr)               kfree(ptr)

#endif

/* Per-device (per-sbm) structure */
struct sbm_dev {
    struct cdev cdev;
    int sbm_number;
    char name[10];
    int status;
    int open_count;
    int is_full;
    struct sbm_config sbm_cfg;

	/* Statistics
	*/
	__u32 wirte_go_cnt;
	__u32 write_mutex_fail_cnt;
	__u32 write_ill_status_cnt;
	__u32 write_ok_cnt;
};

#endif

