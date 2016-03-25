#
# Makefile for the ali Remote Process Call driver (drivers/rpc/Makefile)
#


obj-y += rpchld.o
rpchld-y := ali_rpc_hld_base.o ali_rpc_dev_init.o ali_rpc_init.o

obj-$(CONFIG_RPC_HLD_DECV)  += decv/

obj-$(CONFIG_RPC_HLD_DIS)   += dis/

obj-$(CONFIG_RPC_HLD_DECA)  += deca/

obj-$(CONFIG_RPC_HLD_SND)   += snd/

obj-$(CONFIG_RPC_HLD_GMA)   += gma/

obj-$(CONFIG_RPC_PE)        += pe/

obj-$(CONFIG_RPC_SBM)        += sbm/

obj-$(CONFIG_RPC_HLD_VBI)   += vbi/

obj-$(CONFIG_RPC_HLD_SDEC)  += sdec/

obj-$(CONFIG_RPC_HLD_AVSYNC)  += avsync/

obj-$(CONFIG_RPC_IMAGE)       += image/

obj-$(CONFIG_RPC_ENCODER) 	  += encoder/

EXTRA_CFLAGS += -D__ALI_LINUX_KERNEL__  -I./../../../include/alirpcng
