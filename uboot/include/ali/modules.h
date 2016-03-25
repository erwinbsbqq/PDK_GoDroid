/*****************************************************************************
 *
 *  ALi (Zhuhai) Corporation, All Rights Reserved. 2004 Copyright (C)
 *
 *  File: modules.h
 *
 *  Contents: 	This file define modules remote called.
 *  History:
 *		Date		Author      		Version 	Comment
 *		==========	==================	========== 	=======
 *  1.  05/07/2009  Wen Liu     		0.1.000 	Initial
 *
 *****************************************************************************/
#ifndef __MODULES_H__
#define __MODULES_H__

typedef void (*NORMAL_CALLEE)(UINT8 *);

#define BUILTIN_APPLY_ARGS
#define RPC_MSG_NORMAL    0
#define RPC_MSG_RETURN    1
#define RPC_MSG_WAIT      2
#define RPC_MSG_FAILURE   3

struct remote_call_msg
{
    UINT32 size:30;
    volatile UINT32 flag:2;
    UINT8  func;
    UINT8  pri;
    UINT8  resv;
    UINT8  module;
    UINT32 caller;
    UINT32 retbuf;
    UINT32 parasize;
    UINT32 para0;
    UINT32 para1;
    UINT32 para2;
    UINT32 para3;
};

struct remote_response_msg
{
    UINT32 size:30;
    UINT32 flag:2;
    UINT32 ret;
    UINT32 caller;
    UINT32 retbuf;
};


//Serialize structure 
//被串行化结构的描述
struct desc_storage_struct
{
    UINT16 size;  //size of this structure 结构大小
    UINT8  flag;  //flag of this structure 结构标志
    UINT8  ind;   //index in out para list
    UINT32 off;   //offset of message buffer for storage 结构的存储位置
    UINT32 wp;    //write back address for out parameter
};
//指向被串行化结构的指针的描述
//不支持不对齐的指针
struct desc_pointer_struct
{              
    UINT16 pflag:2; //0: pointer of this structure is a parameter  
                    //1: pointer of this structure is member of another structure
    UINT16 poff:14; //pointer offset if pflag == 1, max struct 16K
                    //index of parameter list if  pflag == 0 
    UINT8  pind;    //index of desc_struct list for pointer 
    UINT8  sind;    //index of desc_struct list for pointed
};

//Attribute of pointer
#define DESC_P_IS_PARA(x)    (((x)&0x1)  == 0)
#define DESC_P_IS_MEMBER(x)  (((x)&0x1)  == 1)
#define DESC_P_IS_RET(x)     (((x)&0x1)  == 0)

#define DESC_STRU_IS_OUTPUT(x) (((x)&0x1) == 1)
#define DESC_STRU_IS_CONSTR(x) (((x)&0x2) == 2)  
#define DESC_STRU_IS_LIST(x)   (((x)&0x4) == 4) 
#define DESC_STRU_IS_DYNAMIC(x)   ((x)&0xe) 

#define TYPE_OUTPUT            1
#define TYPE_STRING            2
#define TYPE_LIST              4
#define TYPE_SIZE_UB           0x10
#define TYPE_SIZE_UH           0x20
#define TYPE_SIZE_UW           0x00
   
#define DESC_P_PARA(ind, pind, sind)       ((pind<<16)|(sind<<24))
#define DESC_P_STRU(ind, pind, sind, poff) ((pind<<16)|(sind<<24)|(poff<<2)|1)
#define DESC_STATIC_STRU(ind, size)        (size), 0, 0
#define DESC_OUTPUT_STRU(ind, size)        (size|0x10000), 0, 0
#define DESC_DYNAMIC_STRU(ind, flag, off)    ((flag)<<16|size), off, 0
#define DESC_STATIC_STRU_SET_SIZE(desc, ind, size) ((UINT32 *)desc)[1 + 3*ind]=size 
#define DESC_OUTPUT_STRU_SET_SIZE(desc, ind, size) ((UINT32 *)desc)[1 + 3*ind]=0x10000|size      
//For example para0->xxx->yyy->zzz
//UINT32 desc_pointer_func1[] = 
//{ 
//  3, DESC_STRU(sizeof(struct), DESC_STRU(sizeof(struct)), DESC_STRU(sizeof(struct)) \
//  3, DESC_P_PARA(0, 0),        DESC_P_STRU(0, 1, off0),   DESC_P_STRU(1, 2, off1)  \
//  0, 
// };
//DESC_DYNAMIC_STRU(0, TYPE_OUTPUT|TYPE_LIST|TYPE_SIZE_UB, offset)
//  jump to void module_caller(msg, fp, funcdesc, desc)
//  return;
//  call_para_serialize(msg,desc,fp, funcdesc)  
//  void OS_RemoteCall(msg, size)
//  return;

#define normal_para_serialize(x)   asm volatile ("sw $5, 4(%0); sw $6, 8(%0); sw $7, 12(%0);"::"r"(x))
#define normal_ret_unserialize(x)  asm volatile ("lw $2, 4(%0);"::"r"(x)) 
#define call_to_func(func, msg) \
                                   asm volatile (".set noreorder;  \
                                                  or  $23, $0, $29; \
                                                  lw  $10, 0x10(%1); \
                                                  sll $10, 2;    \
                                                  subu $29,$10; \
                                                  li   $9, 0x14;\
                                                  addu $9, %1;\
                                                  or  $11, $0, $29; \
                                                  1: beqz  $10, 2f;  \
                                                  nop;             \
                                                  lw   $8, ($9);    \
                                                  sw   $8, ($11); \
                                                  addiu $11, 4;   \
                                                  addiu $9, 4;    \
                                                  b     1b;       \
                                                  addiu $10,-4;   \
                                                  2:             \
                                                  li   $9, 0x14;  \
                                                  or  $16, $0, %1; \
                                                  addu $9, %1;  \
                                                  lw    $4, ($9);  \
                                                  lw    $5, 4($9);  \
                                                  lw    $6, 8($9);  \
                                                  lw    $7, 12($9);  \
                                                  lbu   $8, 0x4($16); \
                                                  sll   $8, 2;  \
                                                  addu  %0, %0, $8; \
                                                  lw    %0, (%0);  \
                                                  jalr  %0;        \
                                                  nop;            \
                                                  or    $29, $0, $23; \
                                                  sw    $2 , 4($16); \
                                                 "::"r"(func),"r"(msg))

//Save all parameters into frame stack and then call function to serialize all into call msg
/*#define jump_to_func(msg, func, para0,funcdesc,desc)  \
                                   do{     \
                                        volatile unsigned long null; \
                                        register unsigned long *fp asm ("$8") = (unsigned long *)&para0; \
	                                 asm volatile ("sw $5, 4(%0); sw $6, 8(%0); sw $7, 12(%0);"::"r"(fp));\
	                                 func(msg, fp,funcdesc,desc);       \
	                                 null = 0; \
                                  }while(0)*/
#define osal_rpc(func_desc, ptr_desc) {unsigned long *fp = __builtin_apply_args(); \
	                                     OS_hld_caller(NULL, fp, func_desc, ptr_desc);\
                                       }  
#define jump_to_func(msg, func, para0,funcdesc,desc)  \
                                   do{     \
                                        unsigned long *fp = __builtin_apply_args();\
                                          func(msg, fp,funcdesc,desc);       \
                                  }while(0)
//__builtin_apply(ssss, __builtin_apply_args(), 7);                                   

void para_serialize(UINT8 *msg, UINT8 *desc, UINT32 *fp, UINT8 parasize);
void ret_serialize(UINT8 *msg, UINT8 *desc, UINT32 ret);
UINT8 *malloc_in_stack(void);
UINT32 *malloc_sm(UINT32 len);
void free_sm(UINT32 *fptr, UINT32 len);
UINT32 OS_hld_caller(UINT8 *msg, UINT32 *fp, UINT32 func_desc, UINT32 *api_desc);
void OS_hld_callee(UINT32 entry_func, UINT8 *msg);

enum REMOTE_MODULES{
HLD_BASE_MODULE,
LLD_TRNG_M36F_MODULE,
LLD_DSC_M36F_MODULE,
LLD_CRYPTO_M36F_MODULE,
HLD_DSC_MODULE,
HLD_CRYPTO_MODULE,
SEE_APP_MODULE,
};


extern UINT32 g_remote_callee[];

#endif	//__MODULES_H__

