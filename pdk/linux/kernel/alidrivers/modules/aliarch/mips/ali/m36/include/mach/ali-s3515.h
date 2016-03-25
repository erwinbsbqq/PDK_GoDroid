/*
 * arch/arm/mach-ali3921/include/mach/ali-s3921.h
 *
 * Generic definitions for ALi S3921 SoC
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <asm/pgtable.h>

#ifndef __ASM_ARCH_ALI_S3515_H
#define __ASM_ARCH_ALI_S3515_H

/* Physical/Virtual base addresses mapping */
#define PHYS_SYSTEM		(0x18000000)	// ALi Soc System IO 		0x1800_0000 - 0x1800_0FFF (4K)
#define PHYS_SOUTHBRIDGE	(0x18018000)	// ALi Soc South Bridge IO 	0x1801_8000 - 0x1801_9FFF (8K) 
#define PHYS_BDMA		(0x18044000)	// ALi Soc South Bridge IO 	0x1804_4000 - 0x1804_4FFF (8K) 
#define PHYS_ARM_PERIPHBASE	(0x1BF00000)	// ARM PERIPHBASE Address for Timer, Watchdogs, Interrupt Controller, SCU register. (16K)
						                    // The PERIPHBASE value should read from CP15 c15 register. 
#define PHYS_NOR_FLASH	(0x0C000000)//ALI NOR flash HW address						                    
						

#define SIZE_ALIIO		0x60000//0x1000000//0x60000//0x1000000//0x1000000 //16M for S3921, HW control reg space.
//#define SIZE_SYSTEM		SZ_4K
#define SIZE_SOUTHBRIDGE	SZ_8K
#define SIZE_ARM_PERIPHBASE	SZ_16K
#define SIZE_NOR_FLASH 0x04000000 //64M
#define SIZE_BDMA		SZ_4K

//#define VIRT_SYSTEM                     (VMALLOC_END - SIZE_ALIIO - SIZE_ARM_PERIPHBASE)
#define VIRT_SYSTEM                     (VMALLOC_END - SIZE_ALIIO)
//#define VIRT_ARM_PERIPHBASE     (VIRT_SYSTEM + SIZE_ALIIO)
#define VIRT_ARM_PERIPHBASE     (VIRT_SYSTEM - SIZE_ARM_PERIPHBASE)
#define VIRT_NOR_FLASH          (VIRT_ARM_PERIPHBASE - SIZE_NOR_FLASH) 


#define VIRT_SOUTHBRIDGE        (VIRT_SYSTEM + (PHYS_SOUTHBRIDGE - PHYS_SYSTEM))

/* Soc System IO Address Mapping */
#define SYS_CHIP_VER		(VIRT_SYSTEM + 0x00)
#define SYS_INT_POL_SELECT1	(VIRT_SYSTEM + 0x28)
#define SYS_INT_POL_SELECT2	(VIRT_SYSTEM + 0x2C)
#define SYS_INT_STATUS1		(VIRT_SYSTEM + 0x30)
#define SYS_INT_STATUS2		(VIRT_SYSTEM + 0x34)
#define SYS_INT_STATUS3		(VIRT_SYSTEM + 0x2A4)
#define SYS_INT_ENABLE1		(VIRT_SYSTEM + 0x38)
#define SYS_INT_ENABLE2		(VIRT_SYSTEM + 0x3C)
#define SYS_INT_ENABLE3		(VIRT_SYSTEM + 0x2A8)
#define ALI_SW_BASE_ADDR 	(VIRT_SYSTEM + 0x2c000)
/* South Birdge Address Mapping */
#define UART1_BASE_ADDR		(VIRT_SOUTHBRIDGE + 0x300)
#define UART2_BASE_ADDR		(VIRT_SOUTHBRIDGE + 0x600)

/* ARM CPU Peripherial Address Mapping */
#define A9_MPCORE_SCU		(VIRT_ARM_PERIPHBASE + 0x0000)
#define A9_MPCORE_GIC_CPU	(VIRT_ARM_PERIPHBASE + 0x0100)
#define A9_MPCORE_GIT		(VIRT_ARM_PERIPHBASE + 0x0200)
#define A9_MPCORE_TWD		(VIRT_ARM_PERIPHBASE + 0x0600)
#define A9_MPCORE_GIC_DIST	(VIRT_ARM_PERIPHBASE + 0x1000)

#define A9_L2_BASE_ADDR		(VIRT_ARM_PERIPHBASE + 0x2000)

//USB Device
#define ALI_IRQ_USB_GADGET   (ALI_SYS_IRQ_BASE + 29)
#define USB_ALI_GADGET_BASE	0x1803D000
#define USB_ALI_GADGET_LEN	0x4A

#define USB_ALI_GADGET_DMA_BASE	0xa0100000
#define USB_ALI_GADGET_DMA_LEN	0x10000

/*USB HOST (EHCI, OHCI)*/
#define ALI_SYS_IRQ_BASE      32     /*added by tony*/
#define ALI_IRQ_USB1_EHCI     (ALI_SYS_IRQ_BASE + 32 + 4)
#define ALI_IRQ_NFLASH        (ALI_SYS_IRQ_BASE + 32 + 16)
#define ALI_IRQ_USB1_OHCI     (ALI_SYS_IRQ_BASE + 32 + 18)
#define ALI_IRQ_USB2_EHCI     (ALI_SYS_IRQ_BASE + 64 + 7)
#define ALI_IRQ_USB2_OHCI     (ALI_SYS_IRQ_BASE + 64 + 6)

/*	Fill SOC HW Register address */
#define ALI_USB_EHCI_IO1	0x1803A000
#define ALI_USB_EHCI_IO2	0x18046000
#define ALI_USB_OHCI_IO1	0x1803A800
#define ALI_USB_OHCI_IO2	0x18046800

#define ALI_USB1_OHCI_PCI_PHY_BASE	ALI_USB_OHCI_IO1
#define ALI_USB1_OHCI_LEN		0x100
#define ALI_USB1_OHCI_PHY_BASE		ALI_USB_OHCI_IO1 + 0x800 
#define ALI_USB1_OHCI_LEN		0x100

#define ALI_USB2_OHCI_PCI_PHY_BASE	ALI_USB_OHCI_IO2
#define ALI_USB2_OHCI_LEN		0x100
#define ALI_USB2_OHCI_PHY_BASE		ALI_USB_OHCI_IO2 + 0x800 
#define ALI_USB2_OHCI_LEN		0x100

#define ALI_USB1_EHCI_PCI_PHY_BASE	ALI_USB_EHCI_IO1
#define ALI_USB1_EHCI_PCI_LEN		0x100
#define ALI_USB1_EHCI_PHY_BASE		ALI_USB_EHCI_IO1 +  0x100
#define ALI_USB1_EHCI_LEN		0x100

#define ALI_USB2_EHCI_PCI_PHY_BASE	ALI_USB_EHCI_IO2
#define ALI_USB2_EHCI_PCI_LEN		0x100
#define ALI_USB2_EHCI_PHY_BASE		ALI_USB_EHCI_IO2 +  0x100
#define ALI_USB2_EHCI_LEN		0x100

#define ALI_USB1_HOST_GENERAL_PURPOSE_BASE		0x1803D800 
#define ALI_USB1_HOST_GENERAL_PURPOSE_LEN		0x30

#define ALI_USB2_HOST_GENERAL_PURPOSE_BASE		0x18049800 
#define ALI_USB2_HOST_GENERAL_PURPOSE_LEN		0x30

// NAND Flash Controller
#define ALI_NANDREG_BASE    0x18032000
#define ALI_NANDREG_LEN     0x60
#define ALI_NANDSRAM_BASE   (ALI_NANDREG_BASE+0x1000)
#define ALI_NANDSRAM_LEN    0x800
#define ALI_NANDBUF_LEN     0x2000 + 0x400
#define ALI_NANDBUF1_BASE   0xa0100000

#define ALI_HWREG_SET_UINT32(val, reg)	((*((volatile u32*)(VIRT_SYSTEM + reg)))=val)
#define ALI_HWREG_GET_UINT32(reg)        (*((volatile u32*)(VIRT_SYSTEM + reg)))


/* For HW buffer configuration. 
*/
#define ALI_MEMALIGNDOWN(a) (a & (~(~PMD_MASK - 1)))
#define ALI_MEMALIGNUP(a)   ((a + ~PMD_MASK - 1) & (~ (~PMD_MASK - 1)))

// SDIO/eMMC Controller
#define SDIO_DRIVER_NAME "ali-mci"
#define ALI_SDREG_BASE    0x18014000
#define ALI_SDREG_LEN     0x60
#define ALI_SDSRAM_BASE   (ALI_SDREG_BASE+0x1000)
#define ALI_SDSRAM_LEN    0x800
#define ALI_IRQ_SDIO     (ALI_SYS_IRQ_BASE + 10)


struct ali_hwbuf_desc
{
    const char *name;    
    unsigned long phy_start;
	unsigned long phy_end;	
};

#endif
