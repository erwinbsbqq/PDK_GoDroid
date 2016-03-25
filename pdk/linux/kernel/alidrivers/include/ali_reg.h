
#ifndef _ALI_REG_H_

#define _ALI_REG_H_

#if defined(CONFIG_ARM)

	#include <mach/ali-s3921.h>
	#define ALI_REGS_PHYS_BASE             	PHYS_SYSTEM 
	#define ALI_REGS_VIRT_BASE             	VIRT_SYSTEM 

#else

	#define ALI_REGS_PHYS_BASE		(0)
	#define ALI_REGS_VIRT_BASE		(0xA0000000)

#endif

#if defined(CONFIG_ARM)
#define __REG8ALI(x)        (*((volatile unsigned char *)((x) - ALI_REGS_PHYS_BASE + \
                                ALI_REGS_VIRT_BASE)))
#define __REG16ALI(x)        (*((volatile unsigned short *)((x) - ALI_REGS_PHYS_BASE + \
									ALI_REGS_VIRT_BASE)))
#define __REG32ALI(x)        (*((volatile unsigned long *)((x) - ALI_REGS_PHYS_BASE + \
									ALI_REGS_VIRT_BASE)))

#define __PREG8ALI(x)        ((volatile unsigned char *)((x) - ALI_REGS_PHYS_BASE + \
                                         ALI_REGS_VIRT_BASE))
#define __PREG16ALI(x)        ((volatile unsigned short *)((x) - ALI_REGS_PHYS_BASE + \
											 ALI_REGS_VIRT_BASE))
#define __PREG32ALI(x)        ((volatile unsigned long *)((x) - ALI_REGS_PHYS_BASE + \
											 ALI_REGS_VIRT_BASE))
#define __REGALIRAW(x)        ((x) - ALI_REGS_PHYS_BASE + \
                               ALI_REGS_VIRT_BASE)
#endif
#if defined(CONFIG_MIPS)
#define ALI_IOCA(x) ((((unsigned long)(x))&0x18ffffff)|0xb8000000)
#define __REG8ALI(x)        (*((volatile unsigned char *)(ALI_IOCA(x))))
#define __REG16ALI(x)        (*((volatile unsigned short *)(ALI_IOCA(x))))
#define __REG32ALI(x)        (*((volatile unsigned long *)(ALI_IOCA(x))))

#define __PREG8ALI(x)        ((volatile unsigned char *)(ALI_IOCA(x)))
#define __PREG16ALI(x)        ((volatile unsigned short *)(ALI_IOCA(x)))
#define __PREG32ALI(x)        ((volatile unsigned long *)(ALI_IOCA(x)))
#define __REGALIRAW(x)        (ALI_IOCA(x))
#endif

/* register read/write operations */
#ifndef readb
#define readb(addr)             (*(volatile unsigned char *)(addr))
#endif

#ifndef readw
#define readw(addr)             (*(volatile unsigned short *)(addr))
#endif

#ifndef readl
#define readl(addr)             (*(volatile unsigned int *)(addr))
#endif

#ifndef writeb
#define writeb(b,addr)          (*(volatile unsigned char *)(addr)) = (b)
#endif

#ifndef writew
#define writew(b,addr)          (*(volatile unsigned short *)(addr)) = (b)
#endif

#ifndef writel
#define writel(b,addr)          (*(volatile unsigned int *)(addr)) = (b)
#endif

#endif

