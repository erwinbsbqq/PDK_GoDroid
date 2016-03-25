#ifndef __ALI_MBX_AS_H
#define __ALI_MBX_AS_H

#define MBX_RCV_MAX_BUF   0x4000          /*The max buffer should be less 16K*/
#define MBX_M2S_BUF_LEN   0x800 //0x80    /*Main to See buffer length, Main->W See->R*/
#define MBX_S2M_BUF_LEN   0x800 //0x40    /*See to Main buffer length, See->W Main->R*/
#define MBX_FREE_BUF_LEN  0x1000 //0x40    /*We remain xx byte space to be used by other modules*/

#define MBX_M2S_BUF_IDX   0x00
#define MBX_S2M_BUF_IDX   MBX_M2S_BUF_IDX + MBX_M2S_BUF_LEN
#define MBX_FREE_BUF_IDX  MBX_S2M_BUF_IDX + MBX_S2M_BUF_LEN

#if defined(__ALI_LINUX_KERNEL__)
   #define MBX_WBUF_IDX   MBX_M2S_BUF_IDX
   #define MBX_WBUF_LEN   MBX_M2S_BUF_LEN
   #define MBX_RBUF_IDX   MBX_S2M_BUF_IDX
   #define MBX_RBUF_LEN   MBX_S2M_BUF_LEN

#else
   #define MBX_WBUF_IDX   MBX_S2M_BUF_IDX
   #define MBX_WBUF_LEN   MBX_S2M_BUF_LEN
   #define MBX_RBUF_IDX   MBX_M2S_BUF_IDX
   #define MBX_RBUF_LEN   MBX_M2S_BUF_LEN
#endif

#define MBX_WBUF_MAX_PKT   MBX_RCV_MAX_BUF/MBX_WBUF_LEN  /*The allowed total packet when sending msg to peer*/


/********************************************************************
*
* -------------------------------------------------------------------
*  |   Len(12bit)  |  Tol Idx(8bit)  |  Cur Idx(8bit)  | Type(4bit) | 
* -------------------------------------------------------------------
*
*********************************************************************/

#define MBX_CTRL_MSG   0x01      /*control message*/
#define MBX_DATA_MSG   0x02      /*data message*/

#define MBX_DATA_LEN_BIT  12     /*data length bit*/
#define MBX_TOL_PKT_BIT   8      /*total index bit*/
#define MBX_CUR_PKT_BIT   8      /*current index bit*/
#define MBX_MSG_TYPE_BIT  4      /*current index bit*/

/*Read control bit content*/
#define MBX_REG_GET_MSG_TYPE(mbx_reg) ((mbx_reg)&0x0F)
#define MBX_REG_GET_CUR_PKT(mbx_reg) ((mbx_reg>>4)&0xFF)
#define MBX_REG_GET_TOL_PKT(mbx_reg) ((mbx_reg>>12)&0xFF) 
#define MBX_REG_GET_LEN(mbx_reg) ((mbx_reg>>20)&0xFFF)

/*Write control bit content*/
#define MBX_REG_WRITE_PREPARE_CONTENT(len, tot, cur, type) ((len<<20)|(tot<<12)|(cur<<4)|(type))

#define MBX_RECEIVED_MASK    0xAA55AAFF    /*Means the peer had received the content*/

/*Error Message*/
#define MBX_ERR_SEND_BUF_COMMON    -0x1000
#define MBX_ERR_SEND_BUF_NULL      -0x1001
#define MBX_ERR_SEND_BUF_OVERFLOW  -0x1002




#endif
