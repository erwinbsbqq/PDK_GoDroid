//#include <common.h>
//#include <asm/arch/sys_proto.h>
//#include <asm/arch/mmc_host_def.h>
//#include <asm/arch/clocks.h>
//#include <asm/arch/gpio.h>
//#include <asm/gpio.h>

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/io.h>
//#include <sys_define.h>

#ifdef CONFIG_USB_EHCI
#include <usb.h>
//#include <asm/arch/ehci.h>
//#include <asm/ehci-omap.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_UBOOT_3921_GMAC
int gmac_register(bd_t *bis);
#endif
#ifdef CONFIG_UBOOT_3921_SWITCH
int sw_register(bd_t *bis);
#endif
#ifdef CONFIG_UBOOT_TOE2
int toe2_register(bd_t *bis);
#endif

/*
const struct omap_sysinfo sysinfo = {
	"Board: C3503 SB/DB\n"
};
*/

phys_size_t initdram(int board_type)
{
	/* Sdram is setup by assembler code */
	/* If memory could be changed, we should return the true value here */
	return ALI3503_MEM_SIZE*1024*1024;	
}

int board_init(void)
{
//	gd->bd->bi_arch_number = MACH_TYPE_ALI_S3921;
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100; /* boot param addr */

	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_UBOOT_3921_GMAC
	gmac_register(bis);
#endif
#ifdef CONFIG_UBOOT_3921_SWITCH
	sw_register(bis);
#endif
#ifdef CONFIG_UBOOT_TOE2
	toe2_register(bis);
#endif
	return 0;
}

int checkboard (void)
{
/*
	u32 proc_id, chip_id;
	u32 cpu_feq, SEE_feq;
    u32 ChipGen = sys_ic_get_chip_id();

	cpu_feq = sys_ic_get_cpu_clock();
	SEE_feq = sys_ic_get_SEE_clock();
	
	proc_id = read_c0_prid();
	chip_id = *((volatile u32 *)(ALI_SOC_BASE));	
    printf ("CPU: MIPS32 , id: 0x%02x, rev: 0x%02x\n",
			   (proc_id >> 8) & 0xFF, proc_id & 0xFF);
	
	printf("Board :");
       if (ALI_S3901 == ChipGen)
       {
        	switch( chip_id & 0xFFFF0300 )
        	{
        		case 0x39010000:	printf("M3911 256Pin NMP 32bits DDR2\n");			break;
        		case 0x39010100:	printf("M3901 216Pin NMP 16bits DDR2\n");			break;			
        		case 0x39010200:	printf("M3701G 256Pin STB with QAM 32bits DDR2\n");	break;
        		case 0x39010300:	printf("M39xx 256Pin STB with CI 16bits DDR2\n");   break;								
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }
       else if (ALI_C3701 == ChipGen){
        	switch( chip_id & 0xFFFF0300 )
        	{
        		case 0x37010300:	printf("M3701C 144Pin STB (%d MHz) with DDR3\n",cpu_feq);   break;								
        		case 0x37010200:	printf("M3701H 256Pin STB (%d MHz) with DDR3\n",cpu_feq);   break;
				case 0x37010100:	printf("M3701C 292Pin STB (%d MHz) with DDR3\n",cpu_feq);   break;
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }else if (ALI_S3503 == ChipGen){
        	switch( chip_id & 0xFFFF0300 )
        	{
        		case 0x35030300:	printf("M3503A 144Pin STB (%d MHz) with DDR3\n",cpu_feq);  break;								
        		case 0x35030200:	printf("M3503A 128Pin STB (%d MHz) with DDR3\n",cpu_feq);  break;
				case 0x35030100:	printf("M3503A 256Pin STB (%d MHz) with DDR3\n",cpu_feq);  break;
				case 0x35030000:	printf("M3503A 292Pin STB (%d MHz) with DDR3\n",cpu_feq);   break;
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
			
       }
       else{
        	switch( chip_id & 0xFFFF0C0F )
        	{
        		case 0x36020002:	printf("M3602B\n");			break;
        		case 0x36030001:	printf("M3601E\n");			break;			
        		case 0x36030401:	printf("M3701E/M3701F\n");	break;
        		case 0x36030801:	printf("M3606/M3606C\n");		break;
        		case 0x36030C01:	printf("M3603\n");			break;									
        		default: 			printf("Unknown (%.4x)\n",chip_id>>16);	break;
        	}
       }
*/
	//printf(" proc_id=0x%x\n", proc_id);

	return 0;
}


/*
 * get_board_rev() - get board revision
 */
u32 get_board_rev(void)
{
	return 0x0101;  //01v01
}

void get_chip_info(u32 *id,u8 *package,u8 *version)
{
	u32 ALi_Chip_Version = readl(ALI_SOC_BASE);
	*id = ALi_Chip_Version >> 16;
	*package = (u8) (ALi_Chip_Version >> 8) & 0x0F;	
	*version = (u8) ALi_Chip_Version & 0xFF;	
}

//0x18001000 bit[2:0] Organization
// 000: 32 MB@32bits
// 001: 64 MB@32bits	
// 010: 128 MB@32bits
// 011: 256 MB@32bits
// 100: 512 MB@32bits	
// 101: 1GB@32bits
// 110: 128MB (DDR2: 8bits x 2, Just 16 bits Interleave Mapping)	
// 111: 1GB@16bits 2GB@32bits			
u32 get_board_mem_size(void)
{
    u32 dm_ctrl_reg,mem_size;
    u32 chip_id,chip_package,chip_ver;
    u8 dram_is_32bit;

    get_chip_info(&chip_id,&chip_package,&chip_ver);

    if (chip_id == C3921)
    {
        dm_ctrl_reg= readl(0x18001000);
        dm_ctrl_reg= dm_ctrl_reg&0x07;
        dram_is_32bit =  (readb(0x18001007)&0x80)>>7;	//0xb8001007 bit[7] 0:16bit 1:32bit 

        switch(dm_ctrl_reg)
        {
            case 0x00:
                mem_size= 32;	//32MB
                break;

            case 0x01:
                mem_size= 64;
                break;

            case 0x02:
                mem_size= 128;
                break;

            case 0x03:
                mem_size= 256;
                break;

            case 0x04:
                mem_size= 512;
                break;

            case 0x05:
                mem_size= 1024;
                break;

            case 0x06: //128MB(DDR2: 8bits x 2, Just 16 bits Interleave Mapping)
                mem_size= 128;
                break;

            case 0x07:
                mem_size= 2048;
                break;

            default:
                mem_size= 512;
                break;
        }

        if(0 == dram_is_32bit && 6 != dm_ctrl_reg)
            mem_size = mem_size>>1;
            
    }

    printf("dm_ctrl_reg:0x%08x,dram_is_32bit=%d,mem_size=%dM\n",dm_ctrl_reg,dram_is_32bit,mem_size);
    return mem_size;
}

/*
 * Initializes on-chip MMC controllers.
 */
int cpu_mmc_init(bd_t *bis)
{
#ifdef CONFIG_ALI_MMC
	return ali_mmc_register(bis);
#else
	return 0;
#endif
}

