#
# Makefile for the ali Remote Process Call driver (drivers/rpc/Makefile)
#


obj-y           += alirpc.o

alirpc-y := ali_rpc_argsformat.o ali_rpcng.o ali_rpc_cfg.o ali_rpc_ctrl.o ali_rpc_debug.o ali_rpc_invoker.o ali_rpc_mcapi.o \
            ali_rpc_osdep.o ali_rpc_servicelisten.o ali_rpc_util.o ali_rpc_xdr.o hashtable.o ali_rpc_service.o ali_minirpc_service.o ali_rpc_platform.o 
EXTRA_CFLAGS += -D__BOOT_SEE_IN_UBOOT__  -I./ 

