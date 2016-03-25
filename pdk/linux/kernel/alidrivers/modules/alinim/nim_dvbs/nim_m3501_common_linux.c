/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:  nim_m3501_common_linux.c
*
*    Description:  s3501/3 common function for linux
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "nim_m3501.h"

RET_CODE nim_send_as_msg(int id, unsigned char lck, unsigned char polar, unsigned short freq, unsigned int sym, unsigned char fec, unsigned char as_stat)
{
    RET_CODE ret = RET_SUCCESS;
    unsigned char msg[13]={0};
    int msg_size = 13;

    AUTOSCAN_PRINTF("%s in\n", __FUNCTION__);
    memset((void *)msg, 0x00, msg_size);
    msg[0] = 0x01;
    msg[1] = 0;
    msg[2] = 9;
    memcpy((void *)(msg + 3), &lck, 1);
    memcpy((void *)(msg + 4), &polar, 1);
    memcpy((void *)(msg + 5), &fec, 1);
    memcpy((void *)(msg + 6), &freq, 2);
    memcpy((void *)(msg + 8), &sym, 4);
    memcpy((void *)(msg + 12), &as_stat, 1);
    ali_transport_send_msg(id, msg, msg_size); //priv->blind_msg.port_id
    return ret;
}


UINT32 dvbs_as_cb2_ui(void *p_priv, unsigned char lck, unsigned char polar, unsigned short freq, unsigned int sym, unsigned char fec, unsigned char as_stat)
{
    PRINTK_INFO("[kangzh]line=%d,%s,enter!\n", __LINE__, __FUNCTION__);
    //    printk("%s in:lock:%d, polar:%d, freq:%d, sym:%d, fec:%d, stop:%d\n",__FUNCTION__,lck, polar,freq,sym,fec,as_stat);
    struct nim_s3501_private *priv = (struct nim_s3501_private *)p_priv;
    priv->as_status = 0;

    nim_send_as_msg(priv->blind_msg.port_id, lck, polar, freq, sym, fec, as_stat);
    wait_event_interruptible_timeout(priv->as_sync_wait_queue, priv->as_status & 0x01, 0x7fffffff);
    priv->as_status = 0;
    PRINTK_INFO("[kangzh]line=%d,%s,end!\n", __LINE__, __FUNCTION__);
    return 0;
}



INT32 nim_callback(NIM_AUTO_SCAN_T *pst_auto_scan, void *pfun, UINT8 status, UINT8 polar, UINT16 freq, UINT32 sym, UINT8 fec, UINT8 stop)
{
    return pst_auto_scan->callback(pfun, status, polar, freq, sym, fec, stop);
}

UINT32 nim_flag_read(struct nim_s3501_private *priv, UINT32 T1, UINT32 T2, UINT32 T3)
{
    UINT32 flag_ptn;

    read_lock(&priv->flagid_rwlk);
    flag_ptn = priv->flag_id;
    read_unlock(&priv->flagid_rwlk);

    return flag_ptn;
}

UINT32 nim_flag_create(struct nim_s3501_private *priv)
{
    if (priv->flag_id == OSAL_INVALID_ID)
    {
        write_lock(&priv->flagid_rwlk);
        priv->flag_id = 0;
        write_unlock(&priv->flagid_rwlk);
    }
    return 0;
}

UINT32 nim_flag_set(struct nim_s3501_private *priv, UINT32 value)
{
    write_lock(&priv->flagid_rwlk);
    priv->flag_id |= value;
    write_unlock(&priv->flagid_rwlk);
    return 0;
}

UINT32 nim_flag_clear(struct nim_s3501_private *priv, UINT32 value)
{
    write_lock(&priv->flagid_rwlk);
    priv->flag_id &= (~value);
    write_unlock(&priv->flagid_rwlk);
    return 0;
}

UINT32 nim_flag_del(struct nim_s3501_private *priv)
{
    write_lock(&priv->flagid_rwlk);
    priv->flag_id = OSAL_INVALID_ID;
    write_unlock(&priv->flagid_rwlk);
    return 0;
}



