#ifndef __ETHERNET_TOE2_UTIL__H
#define __ETHERNET_TOE2_UTIL__H

//#define TOE2_UTILITY_TRACE(msg,args...) printk("TOE2U: " msg "\n", ## args)
#define TOE2_UTILITY_TRACE(msg,args...)
//#define UTIL_DEBUG

typedef struct tagMAC_ADAPTER
{
	unsigned int BaseAddr;
	unsigned int Version;	
	unsigned int InterruptMask;

	unsigned char MacAddr[ETH_ALEN];

	unsigned char *pSetupBuf;
	dma_addr_t pSetupBuf_DMA;
	
	prx_desc pRxDesc;
    dma_addr_t pRxDesc_DMA;
	unsigned int RxDescBuf[TOE2_RX_DESC_NUM];
	dma_addr_t RxDescBuf_DMA[TOE2_RX_DESC_NUM];
	
	ptx_desc pTxDesc;
    dma_addr_t pTxDesc_DMA;
	unsigned int TxDescBuf[MAX_TOE2_PKT_LEN];
	dma_addr_t TxDescBuf_DMA[MAX_TOE2_PKT_LEN];

	//Rx Pkts buf ptr.
	unsigned int RxBuffer;

	//Counter.
	unsigned short RxDescWPtr, RxBufRPtr;
	unsigned short TxDescWPtr;
	
	//current ISR bits. 
	unsigned int CurrentISR;
	//MAC Settings.
	int Duplex, Speed, AutoNeg;
	int RmiiEn;
	int LoopBackEn;
	int HwMdioEn;
	int VlanSupport, VlanTagRemove;
	int PauseFrameRx, PauseFrameTx;    
	//Rx Settings.
	int ToeRxEn;
	int PassMulticast, PassBad, Promiscuous;
	int CrcVerify, ChksumVerify; 
	//Tx Settings.
	int ToeTxEn;
	int TsoEn, UfoEn;
	unsigned short MinMfl, MaxMfl;
	unsigned short MflAuto;
	int TxLenAutoInc;
	unsigned short TxLenAuto;
	unsigned short TxCrcErrs; //Tx Dbg Use.
	//status.
	struct net_device_stats net_stats;
	struct toe2_device_stats mac_stats;

	unsigned short LinkPartner;
	unsigned char FilteringMode;
} MAC_ADAPTER, *PMAC_ADAPTER;

typedef struct tagMAC_Init_Context
{       
	//MAC Settings.
	int Duplex, Speed, AutoNeg;
	int RmiiEn;
	int HwMdioEn;
	int VlanSupport, VlanTagRemove;
	int PauseFrameRx, PauseFrameTx;
	//Rx Settings.
	int ToeRxEn;
} MAC_Init_Context, *PMAC_Init_Context;

typedef struct tagMAC_Rx_Context
{
	//MAC Settings.
	int LoopBackEn;
	//Rx Settings.
	int PassMulticast, PassBad, Promiscuous;
	int CrcVerify, ChksumVerify;
	unsigned char FilteringMode;
} MAC_Rx_Context, *PMAC_Rx_Context;

typedef struct
{
	unsigned char FrameType;
	int AddVlanTag;
	unsigned short TxVlanTag;
	unsigned int DescFrom;
	unsigned int DescTo;
	unsigned int DescLen;
	unsigned int DescTimes;
} MAC_TX_PARA;

typedef struct tagMAC_Tx_Context
{
	//Tx Settings.
	int ToeTxEn;
	int TsoEn, UfoEn;
	unsigned short MinMfl, MaxMfl;
	unsigned short MflAuto;
	int TxLenAutoInc;
	unsigned short TxLenAuto;
	unsigned short TxCrcErrs; //Tx Dbg Use.

	MAC_TX_PARA TxPara;

	unsigned char MacDstAddr[6];
	unsigned char MacSrcAddr[6];
} MAC_Tx_Context, *PMAC_Tx_Context;

typedef struct tagMAC_Status_Context
{
	//status.
	struct net_device_stats net_stats;
	struct toe2_device_stats mac_stats;
	unsigned short TxLenAuto;
	unsigned short TxCrcErrs;
} MAC_Status_Context, *PMAC_Status_Context;

struct tagMAC_transfer_ifr
{
	int cmd_type;
	void *pointer;
};

int util_mac_init(PMAC_Init_Context);
int util_mac_rx_start(PMAC_Rx_Context);
void util_mac_rx_stop(PMAC_ADAPTER);
int util_mac_tx_start(PMAC_Tx_Context);
void util_mac_tx_stop(void);
int util_mac_tx_setup(void);
void util_mac_rx_thread_create(void);
void util_mac_rx_thread_destroy(void);
void util_mac_tx_thread_create(void);
void util_mac_tx_thread_destroy(void);
void util_mac_status(PMAC_Status_Context cxt, PMAC_ADAPTER pAdapter);

#endif
