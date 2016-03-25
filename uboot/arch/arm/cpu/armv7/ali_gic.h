#ifndef _ALI__GIC_H
#define _ALI__GIC_H


#define writel(b,addr) ((*(volatile unsigned long *) (addr)) = (b))
#define readl(addr)    (*(volatile unsigned long *) (addr))

#define DEBUG
#ifdef DEBUG
   #define debug(fmt,args...)  \
   do{ \
	 printf("\nfunc:%s    line:%d   ",__FUNCTION__,__LINE__); \
	 printf(fmt,##args); \
   }while(0)
#else
   #define debug(fmt,args...)
#endif

#define A9_MPCORE_GIC_CPU	0x1BF00100
#define A9_MPCORE_GIC_DIST	0x1BF01000

#define GIC_CPU_CTRL			0x00
#define GIC_CPU_PRIMASK			0x04
#define GIC_CPU_BINPOINT		0x08
#define GIC_CPU_INTACK			0x0c
#define GIC_CPU_EOI			    0x10
#define GIC_CPU_RUNNINGPRI		0x14
#define GIC_CPU_HIGHPRI			0x18

#define GIC_DIST_CTRL			0x000
#define GIC_DIST_CTR			0x004
#define GIC_DIST_ENABLE_SET		0x100
#define GIC_DIST_ENABLE_CLEAR	0x180
#define GIC_DIST_PENDING_SET	0x200
#define GIC_DIST_PENDING_CLEAR	0x280
#define GIC_DIST_ACTIVE_BIT		0x300
#define GIC_DIST_PRI			0x400
#define GIC_DIST_TARGET			0x800
#define GIC_DIST_CONFIG			0xc00
#define GIC_DIST_SOFTINT		0xf00

/* Soc System IO Address Mapping */
#define IO_BASE             0x18000000
#define SYS_CHIP_VER		(IO_BASE + 0x00)
#define SYS_INT_POL_SELECT1	(IO_BASE + 0x28)
#define SYS_INT_POL_SELECT2	(IO_BASE + 0x2C)
#define SYS_INT_STATUS1		(IO_BASE + 0x30)
#define SYS_INT_STATUS2		(IO_BASE + 0x34)
#define SYS_INT_STATUS3		(IO_BASE + 0x2A4)
#define SYS_INT_ENABLE1		(IO_BASE + 0x38)
#define SYS_INT_ENABLE2		(IO_BASE + 0x3C)
#define SYS_INT_ENABLE3		(IO_BASE + 0x2A8)

#define ALI_SYS_IRQ_BASE      32     /*added by tony*/

#define gic_data_dist_base(d)	((d)->dist_base)
#define gic_data_cpu_base(d)	((d)->cpu_base)

enum {
	IRQ_TYPE_NONE		= 0x00000000,
	IRQ_TYPE_EDGE_RISING	= 0x00000001,
	IRQ_TYPE_EDGE_FALLING	= 0x00000002,
	IRQ_TYPE_EDGE_BOTH	= (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING),
	IRQ_TYPE_LEVEL_HIGH	= 0x00000004,
	IRQ_TYPE_LEVEL_LOW	= 0x00000008,
	IRQ_TYPE_LEVEL_MASK	= (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH),
	IRQ_TYPE_SENSE_MASK	= 0x0000000f,

	IRQ_TYPE_PROBE		= 0x00000010,

	IRQ_LEVEL		= (1 <<  8),
	IRQ_PER_CPU		= (1 <<  9),
	IRQ_NOPROBE		= (1 << 10),
	IRQ_NOREQUEST		= (1 << 11),
	IRQ_NOAUTOEN		= (1 << 12),
	IRQ_NO_BALANCING	= (1 << 13),
	IRQ_MOVE_PCNTXT		= (1 << 14),
	IRQ_NESTED_THREAD	= (1 << 15),
	IRQ_NOTHREAD		= (1 << 16),
	IRQ_PER_CPU_DEVID	= (1 << 17),
};

#define IRQF_VALID	(1 << 0)
#define IRQF_PROBE	(1 << 1)
#define IRQF_NOAUTOEN	(1 << 2)

void gic_init(unsigned int irq, void *dist , void *cpu);

#endif  

