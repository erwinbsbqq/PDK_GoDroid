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

LIB	= $(obj)lib$(BOARD).o

COBJS-y += $(BOARD).o

##########################chuhua add
COBJS-y += boot_common.o
COBJS-y += fastCRC.o
COBJS-y += block_sector.o

COBJS-y += bootargs.o
COBJS-y += bcb.o
COBJS-y += deviceinfo.o
COBJS-y += baseparams.o
COBJS-y += boardinfo.o

COBJS-$(CONFIG_NAND_ALI) += ali_3921_part.o ali_nand_enhance.o
COBJS-$(CONFIG_ALI_MMC) += ali_3921_part_emmc.o

COBJS-$(CONFIG_UBOOT_BOOT_SEE) += ali_see_boot.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
