#
# Makefile for the ali shared memory communication driver
#
#
# (C) Copyright 2000-2007
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libminirpc.o

COBJS-y += ali_rpc_argsformat.o ali_rpc_xdr.o ali_minirpc_service.o ali_rpcng.o hashtable.o ali_rpc_cfg.o ali_rpc_ctrl.o ali_rpc_util.o pr_malloc.o ali_rpc_osdep.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

EXTRA_CFLAGS += -D__ALI_LINUX_KERNEL__  -I../inc -I../inc/rpc_hld -I../inc/asm/mach-ali

CFLAGS += $(EXTRA_CFLAGS)
HOSTCFLAGS += $(EXTRA_CFLAGS)
CPPFLAGS += $(EXTRA_CFLAGS)

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
