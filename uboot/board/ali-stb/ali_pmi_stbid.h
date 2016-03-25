
#ifdef CONFIG_NAND_ALI

#define MAX_PMI_SIZE      2048   //pageSize=2048,  1024
#define PartCntOff              (0x100)
#define PartOff                   (0x104)
#define PartNameOff          (0x200)

#define MAX_STBID_LEN    100  //should >= sizeof(STB_ID)
//#define PART_IDX_STBID   2     //0: PMI & Miniboot  1: Aliboot 
//#define PART_IDX_MAIN    6     //see also: ali.ini
//#define PART_IDX_SEE       7     //see also: ali.ini
#define PART_NAME_STBID         "stbid"
#define PART_NAME_MAIN          "kernel"
#define PART_NAME_SEE             "see"
#define PART_NAME_PARTTBL     "parttbl"
#define PART_NAME_SEE_BM       "see_bmedia"
#define PART_NAME_BM              "bootmedia"

struct NAND_CONFIG
{
	unsigned long				wBlksPerChip;			//0x00  Blocks per chip
	unsigned long				wBytesPerPage;			//0x04  Eg. 2K, 4K, 8K/page
	unsigned long				wPagesPerBlock;			//0x08  Eg. 64, 128, 192, 256
	unsigned long				wBytesPerECCSec;		//0x0c  1024
	unsigned long				wBytesPerLBA;			//0x10  512
	unsigned long				bBytesRednt;			//0x14  32 (16 bit ECC) , 46 (24 bit ECC) 
	unsigned long				bBytesPerRow;			//0x18  2, 3, 4 (times of row addr.)
	unsigned long				bECCType;				//0x1c  16bits, 24 bits, 1 bit , no ECC
	unsigned long				wLogEndBlk;				//0x20  Logical end block
	unsigned long				bReadClock;				//0x24  Read clock setting, 0x11, 0x22, 0x33
	unsigned long				bWriteClock;				//0x28  Write clock setting, 0x11, 0x22, 0x33
	unsigned long				wPMPhyEndBlock;		//0x2c  PM physical block
	unsigned long				bBlockShift;				//0x30  Shift bit for block. Could be 5,6, 7 , 8
	unsigned char				bSmReadyMode;			//0x34
	unsigned char				bPollBitMask;			//0x35
	unsigned char				bExpBitMask;			//0x36
	unsigned char				bDelay; 					//0x37
};

typedef struct PMI
{
	unsigned long				Test_Area[3];			//0x00/0, 12Bytes (all 1: 24 bits ECC all 0: 16 bits ECC)
	unsigned long				Signature;				//0x0c/12,   0x3701-55AA
	struct NAND_CONFIG	Config;					//0x10/16, 56Bytes
	unsigned long				Resv[1];					//0x48/72,  Reserved
	unsigned long				DRAM_Init_Start;		//0x4c/76,
	unsigned long				DRAM_Init_Len;			//0x50/80,
	unsigned long				Nand_Bad_Block_Start;	//0x54/84
	unsigned long				Nand_Bad_Block_Len;	//0x58/88
	unsigned long				Security_Data_Start;		//0x5c/92
	unsigned long				Security_Data_Len;		//0x60/96
	unsigned long				Loader_Start;			//0x64/100
	unsigned long				Loader_Len;				//0x68/104
	unsigned long				Resource_Start;			//0x6c/108
	unsigned long				Resource_Len;			//0x70/112
	unsigned long				Linux_Kernel_Start;		//0x74/116
	unsigned long				Linux_Kernel_Len;		//0x78/120

	//						MSO parameter;			//0x200/512 512Bytes
}PMI_t;

#ifdef HAVE_PART_TBL_PARTITION
#define MAX_STBPART_NUM		64
#else
#define MAX_STBPART_NUM		31
#endif

/*
part name:
	aliboot
	stbpart
	stbid		(mac+sn+hw_ver+oui)
	stbinfo
	firstloader
	secondloader
	kernel
	see
	rootfs
	app
	apprsc	(bmp+font)
	appdata	(db+systemdata)
	bootlogo
	radiologo
	...
*/

/* 128 bytes */
typedef struct
{
#ifdef HAVE_PART_TBL_PARTITION
	char name[32];			/* identifier string */
	char fs_type[16];
	unsigned int flash_type;
	unsigned int start;		/* offset within the master MTD space */
	unsigned int size;			/* partition size */
	unsigned int logicstart;			/* partition logic start */
	unsigned int logicsize;			/* partition logic size */
	unsigned int mask_flags;		/* master MTD flags to mask out for this partition */
	char reserved[56];
#else
	unsigned char name[16];
	unsigned long start;
	unsigned long size;
#endif
}PART_INFO;

typedef struct
{
	unsigned int part_num;
	PART_INFO parts[MAX_STBPART_NUM];
#ifdef HAVE_PART_TBL_PARTITION
	char root[1024];
	char mtdparts[2048];
#endif
}PART_TBL;

#define STB_HWINFO_SN_LEN 				26
#define STB_HWINFO_MAC_LEN 			6

typedef struct
{
	unsigned char sn[STB_HWINFO_SN_LEN];
	unsigned char mac[STB_HWINFO_MAC_LEN];
	unsigned long oui;
	unsigned long hw_ver;
}STB_ID;

typedef enum 
{
    STBID_SET_SN = 0,
    STBID_SET_MAC,
    STBID_SET_OUI,
    STBID_SET_HW_VER
}stbid_set_t;

//INT32 stbid_load();
STB_ID *stbid_get(void);
#endif

