
#ifndef _ALI_REG_H_

#define _ALI_REG_H_

#if defined(CONFIG_ALI_CHIP_M3921)

	#include <mach/ali-s3921.h>
	#define ALI_REGS_PHYS_BASE             	PHYS_SYSTEM 
	#define ALI_REGS_VIRT_BASE             	VIRT_SYSTEM 

#else

	#define ALI_REGS_PHYS_BASE		(0)
	#define ALI_REGS_VIRT_BASE		(0xA0000000)

#endif

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

