#
# (C) Copyright 2000, 2001, 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#


include $(TOPDIR)/config.mk

LIB	= $(obj)libminirpchld.o

COBJS-y += ali_rpc_hld_base.o
COBJS-y += ali_rpc_dev_init.o
COBJS-y += ali_rpc_init.o

#COBJS-$(CONFIG_RPC_HLD_TRNG)  += trng/trng_api_remote.o
COBJS-$(CONFIG_RPC_HLD_DSC)  +=dsc/dsc_remote.o
COBJS-$(CONFIG_RPC_HLD_CRYPTO)  += crypto/crypto_remote.o

COBJS-$(CONFIG_RPC_HLD_DECV)  += decv/ali_rpc_hld_decv.o
COBJS-$(CONFIG_RPC_HLD_DIS)   += dis/ali_rpc_hld_dis.o
COBJS-$(CONFIG_RPC_HLD_DECA)  += deca/ali_rpc_hld_gma.o
COBJS-$(CONFIG_RPC_HLD_SND)   += snd/ali_rpc_hld_snd.o
COBJS-$(CONFIG_RPC_HLD_GMA)   += gma/ali_rpc_hld_gma.o
COBJS-$(CONFIG_RPC_PE)        += pe/ali_rpc_pe_cache.o pe/ali_rpc_pe_image.o pe/ali_rpc_pe_music.o pe/ali_rpc_pe_video.o
COBJS-$(CONFIG_RPC_SBM)        += sbm/ali_rpc_sbm.o
COBJS-$(CONFIG_RPC_HLD_VBI)   += vbi/ali_rpc_hld_vbi.o
COBJS-$(CONFIG_RPC_HLD_SDEC)  += sdec/ali_rpc_hld_sdec.o
COBJS-$(CONFIG_RPC_HLD_AVSYNC)  += avsync/ali_rpc_hld_avsync.o
COBJS-$(CONFIG_RPC_IMAGE)       += image/ali_rpc_image.o


COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))


EXTRA_CFLAGS += -D__ALI_LINUX_KERNEL__  -I../inc -I../inc/rpc_hld -I../inc/asm/mach-ali -I../inc/linux

CFLAGS += $(EXTRA_CFLAGS)
HOSTCFLAGS += $(EXTRA_CFLAGS)
CPPFLAGS += $(EXTRA_CFLAGS)

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

