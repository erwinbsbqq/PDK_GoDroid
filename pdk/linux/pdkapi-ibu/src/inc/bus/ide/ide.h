/*
 * Copyright (C) ALi Shanghai Corp. 2005
 *
 * FileName	: ide.h
 * Description	: IDE device generic interface definitions.
 * History	:
 *   Date	Version		Author		Comment
 *   ========	========	========	==========
 * 1.20050523	0.1		Zhengdao Li	Code re-format.
 */
#ifndef _IDE_H
#define _IDE_H

#include <types.h>
#include <sys_config.h>

#include <osal/osal.h>

#include <bus/ide/hdreg.h>

#define IDE_PRINTF(...)		do{}while(0)//PRINTF

#define time_after(a, b)	\
	((INT32)(b)-(INT32)(a)<0)
#define time_before(a, b)	time_after(b, a)

#define time_after_eq(a, b)	\
	((INT32)(b)-(INT32)(a)<=0)
#define time_before_eq(a, b)	time_after_eq(b, a)	

#define INITIAL_MULT_COUNT		0

#define HWIF(drive)	((hwif_t *)((drive)->hwif))

#define IDE_NR_PORTS	(10)

#define IDE_DATA_OFFSET		(0)
#define IDE_ERROR_OFFSET	(1)
#define IDE_NSECTOR_OFFSET	(2)
#define IDE_SECTOR_OFFSET	(3)
#define IDE_LCYL_OFFSET		(4)
#define IDE_HCYL_OFFSET		(5)
#define IDE_SELECT_OFFSET	(6)
#define IDE_STATUS_OFFSET	(7)
#define IDE_CONTROL_OFFSET	(8)
#define IDE_IRQ_OFFSET		(9)

#define IDE_FEATURE_OFFSET	IDE_ERROR_OFFSET
#define IDE_COMMAND_OFFSET	IDE_STATUS_OFFSET

#define IDE_DATA_REG		(HWIF(drive)->io_ports[IDE_DATA_OFFSET])
#define IDE_ERROR_REG		(HWIF(drive)->io_ports[IDE_ERROR_OFFSET])
#define IDE_NSECTOR_REG		(HWIF(drive)->io_ports[IDE_NSECTOR_OFFSET])
#define IDE_SECTOR_REG		(HWIF(drive)->io_ports[IDE_SECTOR_OFFSET])
#define IDE_LCYL_REG		(HWIF(drive)->io_ports[IDE_LCYL_OFFSET])
#define IDE_HCYL_REG		(HWIF(drive)->io_ports[IDE_HCYL_OFFSET])
#define IDE_SELECT_REG		(HWIF(drive)->io_ports[IDE_SELECT_OFFSET])
#define IDE_STATUS_REG		(HWIF(drive)->io_ports[IDE_STATUS_OFFSET])
#define IDE_CONTROL_REG		(HWIF(drive)->io_ports[IDE_CONTROL_OFFSET])
#define IDE_IRQ_REG		(HWIF(drive)->io_ports[IDE_IRQ_OFFSET])

#define IDE_FEATURE_REG		IDE_ERROR_REG
#define IDE_COMMAND_REG		IDE_STATUS_REG
#define IDE_ALTSTATUS_REG	IDE_CONTROL_REG
#define IDE_IREASON_REG		IDE_NSECTOR_REG
#define IDE_BCOUNTL_REG		IDE_LCYL_REG
#define IDE_BCOUNTH_REG		IDE_HCYL_REG

#define OK_STAT(stat,good,bad)	(((stat)&((good)|(bad)))==(good))
#define BAD_R_STAT		(BUSY_STAT   | ERR_STAT)
#define BAD_W_STAT		(BAD_R_STAT  | WRERR_STAT)
#define BAD_STAT		(BAD_R_STAT  | DRQ_STAT)
#define DRIVE_READY		(READY_STAT  | SEEK_STAT)
#define DATA_READY		(DRQ_STAT)

#define BAD_CRC			(ABRT_ERR    | ICRC_ERR)


#define MAX_DRIVES	2	/* per interface; 2 assumed by lots of code */
#define SECTOR_SIZE	512
#define SECTOR_WORDS	(SECTOR_SIZE / 4)	/* number of 32bit words per sector */
#define IDE_LARGE_SEEK(b1,b2,t)	(((b1) > (b2) + (t)) || ((b2) > (b1) + (t)))

/*
 * Timeouts for various operations:
 */
#if 1 
/* slow profile */
#define WAIT_DRQ	3000		/* 100msec - spec allows up to 20ms */
#define WAIT_READY	3000		/* 100msec - should be instantaneous */
#define WAIT_PIDENTIFY	10000		/* 10sec  - should be less than 3ms (?), if all ATAPI CD is closed at boot */
#define WAIT_WORSTCASE	30000		/* 30sec  - worst case when spinning up */
#define WAIT_CMD	1000		/* 10sec  - maximum wait for an IRQ to happen */
#define WAIT_MIN_SLEEP	20		/* 20msec - minimum sleep time */
#else
#define WAIT_DRQ	100		/* 100msec - spec allows up to 20ms */
#define WAIT_READY	100		/* 100msec - should be instantaneous */
#define WAIT_PIDENTIFY	10000		/* 10sec  - should be less than 3ms (?), if all ATAPI CD is closed at boot */
#define WAIT_WORSTCASE	30000		/* 30sec  - worst case when spinning up */
#define WAIT_CMD	1000		/* 10sec  - maximum wait for an IRQ to happen */
#define WAIT_MIN_SLEEP	20		/* 20msec - minimum sleep time */
#endif

/*
 * Now for the data we need to maintain per-drive:  ide_drive_t
 */

#define ide_scsi	0x21
#define ide_disk	0x20
#define ide_optical	0x7
#define ide_cdrom	0x5
#define ide_tape	0x1
#define ide_floppy	0x0

#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
#define __LITTLE_ENDIAN_BITFIELD
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)
#define __BIG_ENDIAN_BITFIELD
#endif

/*
 * Special Driver Flags
 *
 * set_geometry	: respecify drive geometry
 * recalibrate	: seek to cyl 0
 * set_multmode	: set multmode count
 * set_tune	: tune interface for drive
 * serviced	: service command
 * reserved	: unused
 */
typedef union {
	unsigned all			: 8;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned set_geometry	: 1;
		unsigned recalibrate	: 1;
		unsigned set_multmode	: 1;
		unsigned set_tune	: 1;
		unsigned serviced	: 1;
		unsigned reserved	: 3;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned reserved	: 3;
		unsigned serviced	: 1;
		unsigned set_tune	: 1;
		unsigned set_multmode	: 1;
		unsigned recalibrate	: 1;
		unsigned set_geometry	: 1;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} special_t;

/*
 * ATA DATA Register Special.
 * ATA NSECTOR Count Register().
 * ATAPI Byte Count Register.
 * Channel index ordering pairs.
 */
typedef union {
	unsigned all			:16;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned low		:8;	/* LSB */
		unsigned high		:8;	/* MSB */
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned high		:8;	/* MSB */
		unsigned low		:8;	/* LSB */
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} ata_nsector_t, ata_data_t, atapi_bcount_t, ata_index_t;

/*
 * ATA-IDE Error Register
 *
 * mark		: Bad address mark
 * tzero	: Couldn't find track 0
 * abrt		: Aborted Command
 * mcr		: Media Change Request
 * id		: ID field not found
 * mce		: Media Change Event
 * ecc		: Uncorrectable ECC error
 * bdd		: dual meaing
 */
typedef union {
	unsigned all			:8;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned mark		:1;
		unsigned tzero		:1;
		unsigned abrt		:1;
		unsigned mcr		:1;
		unsigned id		:1;
		unsigned mce		:1;
		unsigned ecc		:1;
		unsigned bdd		:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned bdd		:1;
		unsigned ecc		:1;
		unsigned mce		:1;
		unsigned id		:1;
		unsigned mcr		:1;
		unsigned abrt		:1;
		unsigned tzero		:1;
		unsigned mark		:1;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} ata_error_t;

/*
 * ATA-IDE Select Register, aka Device-Head
 *
 * head		: always zeros here
 * unit		: drive select number: 0/1
 * bit5		: always 1
 * lba		: using LBA instead of CHS
 * bit7		: always 1
 */
typedef union {
	UINT8 all;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		UINT8 head		: 4;
		UINT8 unit		: 1;
		UINT8 bit5		: 1;
		UINT8 lba		: 1;
		UINT8 bit7		: 1;
#elif defined(__BIG_ENDIAN_BITFIELD)
		UINT8 bit7		: 1;
		UINT8 lba		: 1;
		UINT8 bit5		: 1;
		UINT8 unit		: 1;
		UINT8 head		: 4;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
}__attribute__((packed)) select_t, ata_select_t;

/*
 * The ATA-IDE Status Register.
 * The ATAPI Status Register.
 *
 * check	: Error occurred
 * idx		: Index Error
 * corr		: Correctable error occurred
 * drq		: Data is request by the device
 * dsc		: Disk Seek Complete			: ata
 *		: Media access command finished		: atapi
 * df		: Device Fault				: ata
 *		: Reserved				: atapi
 * drdy		: Ready, Command Mode Capable		: ata
 *		: Ignored for ATAPI commands		: atapi
 * bsy		: Disk is Busy
 *		: The device has access to the command block
 */
typedef union {
	unsigned all			:8;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned check		:1;
		unsigned idx		:1;
		unsigned corr		:1;
		unsigned drq		:1;
		unsigned dsc		:1;
		unsigned df		:1;
		unsigned drdy		:1;
		unsigned bsy		:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned bsy		:1;
		unsigned drdy		:1;
		unsigned df		:1;
		unsigned dsc		:1;
		unsigned drq		:1;
		unsigned corr           :1;
		unsigned idx		:1;
		unsigned check		:1;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} ata_status_t, atapi_status_t;

/*
 * ATA-IDE Control Register
 *
 * bit0		: Should be set to zero
 * nIEN		: device INTRQ to host
 * SRST		: host soft reset bit
 * bit3		: ATA-2 thingy, Should be set to 1
 * reserved456	: Reserved
 * HOB		: 48-bit address ordering, High Ordered Bit
 */
typedef union {
	unsigned all			: 8;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned bit0		: 1;
		unsigned nIEN		: 1;
		unsigned SRST		: 1;
		unsigned bit3		: 1;
		unsigned reserved456	: 3;
		unsigned HOB		: 1;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned HOB		: 1;
		unsigned reserved456	: 3;
		unsigned bit3		: 1;
		unsigned SRST		: 1;
		unsigned nIEN		: 1;
		unsigned bit0		: 1;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} ata_control_t;

/*
 * ATAPI Feature Register
 *
 * dma		: Using DMA or PIO
 * reserved321	: Reserved
 * reserved654	: Reserved (Tag Type)
 * reserved7	: Reserved
 */
typedef union {
	unsigned all			:8;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned dma		:1;
		unsigned reserved321	:3;
		unsigned reserved654	:3;
		unsigned reserved7	:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned reserved7	:1;
		unsigned reserved654	:3;
		unsigned reserved321	:3;
		unsigned dma		:1;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} atapi_feature_t;

/*
 * ATAPI Interrupt Reason Register.
 *
 * cod		: Information transferred is command (1) or data (0)
 * io		: The device requests us to read (1) or write (0)
 * reserved	: Reserved
 */
typedef union {
	unsigned all			:8;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned cod		:1;
		unsigned io		:1;
		unsigned reserved	:6;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned reserved	:6;
		unsigned io		:1;
		unsigned cod		:1;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} atapi_ireason_t;

/*
 * The ATAPI error register.
 *
 * ili		: Illegal Length Indication
 * eom		: End Of Media Detected
 * abrt		: Aborted command - As defined by ATA
 * mcr		: Media Change Requested - As defined by ATA
 * sense_key	: Sense key of the last failed packet command
 */
typedef union {
	unsigned all			:8;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned ili		:1;
		unsigned eom		:1;
		unsigned abrt		:1;
		unsigned mcr		:1;
		unsigned sense_key	:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned sense_key	:4;
		unsigned mcr		:1;
		unsigned abrt		:1;
		unsigned eom		:1;
		unsigned ili		:1;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} atapi_error_t;

/*
 * ATAPI floppy Drive Select Register
 *
 * sam_lun	: Logical unit number
 * reserved3	: Reserved
 * drv		: The responding drive will be drive 0 (0) or drive 1 (1)
 * one5		: Should be set to 1
 * reserved6	: Reserved
 * one7		: Should be set to 1
 */
typedef union {
	unsigned all			:8;
	struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
		unsigned sam_lun	:3;
		unsigned reserved3	:1;
		unsigned drv		:1;
		unsigned one5		:1;
		unsigned reserved6	:1;
		unsigned one7		:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
		unsigned one7		:1;
		unsigned reserved6	:1;
		unsigned one5		:1;
		unsigned drv		:1;
		unsigned reserved3	:1;
		unsigned sam_lun	:3;
#else
#error "Please fix <sys_config.h>"
#endif
	} b;
} atapi_select_t;

/*
 * Status returned from various ide_ functions
 */
typedef enum {
	ide_stopped,	/* no drive operation was started */
	ide_started,	/* a drive operation was started, handler was set */
} ide_startstop_t;

enum ide_pm_stat {
	ide_pm_wakeup,
	ide_pm_standy,
	ide_pm_sleep,
};

typedef struct ide_drive_s {
	UINT8 name[4];
	struct hd_driveid *id;
	struct ide_drive_s *next;
	struct hwif_s *hwif;
	select_t select;
	UINT32 present		: 1;	/* drive is physically present */
	UINT32 dead		: 1;	/* device ejected hint */
	UINT32 id_read		: 1;	/* 1=id read from disk 0 = synthetic */
	UINT32 noprobe 		: 1;	/* from:  hdx=noprobe */
	UINT32 removable	: 1;	/* 1 if need to do check_media_change */
	UINT32 attach		: 1;	/* needed for removable devices */
	UINT32 is_flash		: 1;	/* 1 if probed as flash */
	UINT32 forced_geom	: 1;	/* 1 if hdx=c,h,s was given at boot */
	UINT32 no_unmask	: 1;	/* disallow setting unmask bit */
	UINT32 no_io_32bit	: 1;	/* disallow enabling 32bit I/O */
	UINT32 atapi_overlap	: 1;	/* ATAPI overlap (not supported) */
	UINT32 nice0		: 1;	/* give obvious excess bandwidth */
	UINT32 nice2		: 1;	/* give a share in our own bandwidth */
	UINT32 doorlocking	: 1;	/* for removable only: door lock/unlock works */
	UINT32 autotune		: 2;	/* 0=default, 1=autotune, 2=noautotune */
	UINT32 remap_0_to_1	: 1;	/* 0=noremap, 1=remap 0->1 (for EZDrive) */
	UINT32 blocked        	: 1;	/* 1=powermanagment told us not to do anything, so sleep nicely */
	UINT32 vdma		: 1;	/* 1=doing PIO over DMA 0=doing normal DMA */
	UINT32 addressing	: 3;	/* 3;
					 *  0=28-bit
					 *  1=48-bit
					 *  2=48-bit doing 28-bit
					 *  3=64-bit
					 */
	UINT32 scsi		: 1;	/* 0=default, 1=ide-scsi emulation */

        UINT8 suspend_reset;	/* drive suspend mode flag, soft-reset recovers */
        UINT8 init_speed;	/* transfer rate set at boot */
        UINT8 pio_speed;      	/* unused by core, used by some drivers for fallback from DMA */
        UINT8 current_speed;	/* current transfer rate set */
	UINT8 dn;		/* now wide spread use */
        UINT8 wcache;		/* status of write cache */
	UINT8 acoustic;		/* acoustic management */
	UINT8 media;		/* disk, cdrom, tape, floppy, ... */
	UINT8 ctl;		/* "normal" value for IDE_CONTROL_REG */
	UINT8 ready_stat;	/* min status value for drive ready */
	UINT8 mult_count;	/* current multiple sector setting */
	UINT8 mult_req;		/* requested multiple sector setting */
	UINT8 tune_req;		/* requested drive tuning setting */
	UINT8 io_32bit;		/* 0=16-bit, 1=32-bit, 2/3=32bit+sync */
	UINT8 bad_wstat;	/* used for ignoring WRERR_STAT */
	UINT8 nowerr;		/* used for ignoring WRERR_STAT */
	UINT8 sect0;		/* offset of first sector for DM6:DDO */
	UINT8 head;		/* "real" number of heads */
	UINT8 sect;		/* "real" sectors per track */
	UINT8 bios_head;	/* BIOS/fdisk/LILO number of heads */
	UINT8 bios_sect;	/* BIOS/fdisk/LILO sectors per track */
	UINT8 doing_barrier;	/* state, 1=currently doing flush */

	UINT32 bios_cyl;	/* BIOS/fdisk/LILO number of cyls */
	UINT32 cyl;		/* "real" number of cyls */
	UINT32 drive_data;	/* use by tuneproc/selectproc */
	UINT32 usage;		/* current "open()" count for drive */
	UINT32 failures;	/* current failure count */
	UINT32 max_failures;	/* maximum allowed failure count */

	unsigned long long capacity64;	/* total number of sectors */

	INT32 lun;		/* logical unit */
	INT32 crc_count;	/* crc counter to reduce drive speed */	
	UINT32 power_stat;
} ide_drive_t;

typedef struct hwif_s {
	UINT8 name[6];
	UINT8 io_ports[IDE_NR_PORTS];
	struct ide_drive_s drives[MAX_DRIVES];
	OSAL_ID semaphore;
	void (*ata_input_data)(ide_drive_t *, void *, UINT32);
	void (*ata_output_data)(ide_drive_t *, void *, UINT32);

	UINT8 (*INB)(UINT8 port);
	UINT16 (*INW)(UINT8 port);
	UINT32 (*INL)(UINT8 port);
	void (*INSW)(UINT8 port, void *addr, UINT32 count);
	void (*INSL)(UINT8 port, void *addr, UINT32 count);
	
	void (*OUTB)(UINT8 value, UINT8 port);
	void (*OUTW)(UINT16 value, UINT8 port);
	void (*OUTL)(UINT32 value, UINT8 port);
	void (*OUTSW)(UINT8 port, void *addr, UINT32 count);
	void (*OUTSL)(UINT8 port, void *addr, UINT32 count);

#if 1//(SYS_CHIP_MODULE==ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)	// eagle for S3601
	INT32 (*start_dma)(ide_drive_t *, UINT32 addr, UINT32 len, INT32 direction,UINT8 mode);
	INT32 (*wait_dma_over)(ide_drive_t *,INT32 direction,UINT32 timeout,UINT32 mode);
#else
	void (*start_dma)(ide_drive_t *, UINT32 addr, UINT32 len, INT32 direction);
	INT32 (*wait_dma_over)(ide_drive_t *);
#endif	

} hwif_t;

#if 0
typedef ide_startstop_t (ide_pre_handler_t)(ide_drive_t *, struct request *);
typedef ide_startstop_t (ide_handler_t)(ide_drive_t *);
typedef INT32 (ide_expiry_t)(ide_drive_t *);
#endif

typedef struct ide_task_s {
/*
 *	struct hd_drive_task_hdr	tf;
 *	task_struct_t		tf;
 *	struct hd_drive_hob_hdr		hobf;
 *	hob_struct_t		hobf;
 */
	task_ioreg_t		tfRegister[8];
	task_ioreg_t		hobRegister[8];
	ide_reg_valid_t		tf_out_flags;
	ide_reg_valid_t		tf_in_flags;
	int			data_phase;
	int			command_type;
#if 0	
	ide_pre_handler_t	*prehandler;
	ide_handler_t		*handler;
	struct request		*rq;		/* copy of request */
	void			*special;	/* valid_t generally */
#endif	
} ide_task_t;

/* check if CACHE FLUSH (EXT) command is supported (bits defined in ATA-6) */
#define ide_id_has_flush_cache(id)	((id)->cfs_enable_2 & 0x3000)

#define ide_id_has_flush_cache_ext(id)	\
	(((id)->cfs_enable_2 & 0x2400) == 0x2400)

#define do_div(n, base) ({ \
	unsigned long long __quot; \
	unsigned long __mod; \
	unsigned long long __div; \
	unsigned long __upper, __low, __high, __base; \
	\
	__div = (n); \
	__base = (base); \
	\
	__high = __div >> 32; \
	__low = __div; \
	__upper = __high; \
	\
	if (__high) \
		__asm__("divu	$0, %z2, %z3" \
			: "=h" (__upper), "=l" (__high) \
			: "Jr" (__high), "Jr" (__base) \
			: GCC_REG_ACCUM); \
	\
	__mod = do_div64_32(__low, __upper, __low, __base); \
	\
	__quot = __high; \
	__quot = __quot << 32 | __low; \
	(n) = __quot; \
	__mod; })


#define RQ_FLAG_DATA_IN			1
#define RQ_FLAG_DATA_OUT		2

#if 1//(SYS_CHIP_MODULE==ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)	// eagle for s3601
enum {
	IDE_INTR_ATA_INT2B			= 0x01,
	IDE_INTR_TXEND_INT			= 0x02,
	IDE_INTR_TSTOP_INT			= 0x04,
	IDE_INTR_SYNC_ATAINTRQ		= 0x08,
	IDE_INTR_CLR_TOINT			= 0x10,
};

#define IDE_FLAG_ATA_INT2B 		IDE_INTR_ATA_INT2B
#define IDE_FLAG_TXEND_INT			IDE_INTR_TXEND_INT
#define IDE_FLAG_TSTOP_INT			IDE_INTR_TSTOP_INT
#define IDE_FLAG_SYNC_ATAINTRQ	IDE_INTR_SYNC_ATAINTRQ
#define IDE_FLAG_CLR_TOINT			IDE_INTR_CLR_TOINT
/*
enum {
	IDE_FLAG_ATA_INT2B			= 0x01,
	IDE_FLAG_TXEND_INT			= 0x02,
	IDE_FLAG_TSTOP_INT			= 0x04,
	IDE_FLAG_SYNC_ATAINTRQ		= 0x08,
	IDE_FLAG_CLR_TOINT			= 0x10,
};
*/
#else
enum {
	IDE_INTR_ATA			= 0x01,
	IDE_INTR_DMA			= 0x02,
	IDE_INTR_WDMA			= 0x04,
};

enum {
	IDE_FLAG_INTRQ			= 0x00000001,
	IDE_FLAG_DMA			= 0x00000002,
	IDE_FLAG_WDMA			= 0x00000004,
};
#endif

struct ide_mutex
{
	void (*lock) (void);
	void (*unlock) (void);
};

void ide_mutex_init(void); 
INT32 ide_exec_command(ide_drive_t *drive, UINT8 cmd, UINT8 feature, UINT8 nsector);
INT32 ide_s3601_init();
unsigned long long hdd_disk_capacity (void);
INT32 hdd_read_sector(UINT32 block, UINT8 *buffer,UINT16 sector_nr);
INT32 hdd_write_sector(UINT32 block, UINT8 *buffer,UINT16 sector_nr);

#endif /* _IDE_H */
