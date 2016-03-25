#
# (C) Copyright 2003-2006
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o 

COBJS-y += ali-stb.o
COBJS-$(CONFIG_ALI_TOE2) += ethernet_mac.o
COBJS-$(CONFIG_ALI_TOE2) += ethernet_mii.o
COBJS-$(CONFIG_USB_EHCI_ALI) += ehci-ali.o
COBJS-$(CONFIG_USB_OHCI_ALI) += ohci-ali.o

COBJS-$(CONFIG_NAND_ALI) += ali_pmi_stbid.o
COBJS-$(CONFIG_NAND_ALI) += ali_nand.o

COBJS-$(CONFIG_NORFLASH_ALI) += norflash/flash.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/flash_data_s.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/flash_raw_sl_c.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/flash_sl.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/hld_dev.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/sto.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/chunk_sto.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/flash_data.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/sto_flash.o
COBJS-$(CONFIG_NORFLASH_ALI) += norflash/flash_sst26_sl_c.o

ifndef ALI_SOC_M36XX
SOBJS	= lowlevel_init.o
else
SOBJS	= lowlevel_init_m36xx.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
