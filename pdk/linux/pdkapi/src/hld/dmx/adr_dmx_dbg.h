#ifndef __ADR_DMX_DBG_H__
#define __ADR_DMX_DBG_H__

#define DMX_DBG_CMD_NR		10
#define DMX_DBG_PRINT_INTRV	5000

typedef enum dmx_dbg_level
{
	DMX_DBG_LEVEL_HLD = 0x1,
    DMX_DBG_LEVEL_KERNEL = 0x2,
    DMX_DBG_LEVEL_SEE = 0x4,
} DMX_DBG_LEVEL;

typedef struct dmx_dbg_stat_ctrl
{
	int KernGlbFd;
	int SeeGlbFd;
	int HwRegFd;

	UINT32 TaskId;

	UINT8 DmxDbgInit;

    UINT8 DmxDbgStatEn;
	UINT8 KernGlbStatEn;
	UINT8 SeeGlbStatEn;
    UINT8 IoCmdStatEn;
    UINT8 VidStatEn;
    UINT8 AudStatEn;
    UINT8 PcrStatEn;
    UINT8 SecStatEn;
    UINT8 PesStatEn;
    UINT8 TsStatEn;
	UINT8 MiscStatEn;
    UINT8 HwRegDumpEn;
    UINT32 DbgShowIntv; /* Unit: ms, default to 3000ms. */
	UINT32 DbgShowTms;

	DMX_DBG_LEVEL DbgLevel;
} dmx_dbg_stat_ctrl;

void dmx_dbg_init(void);
void dmx_dbg_exit(char *);

#endif /* __ADR_DMX_DBG_H__ */
