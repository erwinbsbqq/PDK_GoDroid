#
# Makefile for the kernel DVB device drivers.
#

obj-$(CONFIG_DVB_ALI_M3602_DMX) += alidmx.o

alidmx-y := dmx_from_hw.o dmx_from_app.o dmx_see_interface.o\
            dmx_ts_service.o dmx_ts2pes_service.o dmx_ts2sec_service.o \
			dmx_pcr_service.o dmx_data_pkt.o dmx_linux_api.o dmx_statistics.o \
			dmx_hw.o dmx_hw_m36f.o dmx_hw_m36g.o

EXTRA_CFLAGS += -I./ -Idrivers/media/dvb/dvb-core/ -Idrivers/media/dvb/frontends/

