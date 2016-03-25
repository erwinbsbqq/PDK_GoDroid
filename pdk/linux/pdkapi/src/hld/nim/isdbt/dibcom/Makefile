CFLAGS  = -g -Wall -Wsign-compare

LIBS    = -lm -lpthread
INCLUDE = -I. -Iinclude

# from sample
-include Makefile.include.common

-include ../driver/Makefile.include.common
-include ../driver/Makefile.include.dib7000m_common
-include ../driver/Makefile.include.dib7000
-include ../driver/Makefile.include.dib9080
-include ../driver/Makefile.include.dib9090
-include ../driver/Makefile.include.dib9000_common
-include ../driver/Makefile.include.dib8000
-include ../driver/Makefile.include.dib0070
-include ../driver/Makefile.include.dib0080
-include ../driver/Makefile.include.dib0090
-include ../driver/Makefile.include.dib7070
-include ../driver/Makefile.include.dib8070
-include ../driver/Makefile.include.dib8090
-include ../driver/Makefile.include.external_tunner_common
-include ../driver/Makefile.include.mt2x6x
-include ../driver/Makefile.include.mt2060
-include ../driver/Makefile.include.mt2067
-include ../driver/Makefile.include.max3541
-include ../driver/Makefile.include.dib0190
-include ../driver/Makefile.include.xc5000

# all objects so far are in ../driver so prepend ../driver for object-path

OBJECTS =  $(addprefix ../driver/, $(DIBCOM_DRIVER_OBJECTS))
INCLUDE += -I../driver/include

OBJECTS += $(DIBCOM_SAMPLE_OBJECTS)
ifdef HAVE_EXTERNAL_TUNNER_COMMON
  OBJECTS += sample_tune.o board/layout/demod_parameters.o
  ifdef HAVE_DIB7000
  	TARGETS += sample_application_0 sample_application_1 sample_application_2 sample_application_3
  endif
endif

ifdef HAVE_DIB7000
  ifdef HAVE_MT2060
  	TARGETS += sample_nim7000_d2_mt2060
  endif

  ifdef HAVE_MAX3541
    TARGETS += sample_nim7000_d2_max3541
  endif

  ifdef HAVE_MT2067
    TARGETS += sample_nim7000_d3_mt2067
  endif

  ifdef HAVE_DIB7070
    TARGETS += sample_nim7070p sample_nim7070md sample_nim7070md_gpio12 sample_tfe7070q sample_tfe7070md4 sample_tfe7070pvr
  endif
endif

ifdef HAVE_DIB8000
  ifdef HAVE_MT2060
  	TARGETS += sample_nim8000_d2_mt2060
  endif
  ifdef HAVE_MT2067
  	TARGETS += sample_nim8000_d2_mt2067
  endif
  ifdef HAVE_DIB8070
    TARGETS += sample_tfe8070md4 sample_tfe8070md2
  endif
  ifdef HAVE_XC5000
    TARGETS += sample_nim8006gph
  endif
endif

ifdef HAVE_DIB9080
  TARGETS += sample_nim9080m sample_nim9080md sample_nim9080md_hb_1_8V
endif

ifdef HAVE_DIB9090
  TARGETS += sample_nim9090md sample_nim9090m
endif

ifdef HAVE_DIB8090
  TARGETS += sample_nim8096md sample_tfe8096gp sample_tfe8096md4 sample_tunetime
endif

TARGET_OBJECTS = $(addsuffix .o, $(TARGETS))

all: $(TARGET_OBJECTS) $(TARGETS)

%.o : %.c Makefile include/platform/config.h
	@echo "  [CC]  $@"
	@gcc $(CFLAGS) $(INCLUDE) -c -o $@ $<

$(TARGETS): $(OBJECTS) $(TARGET_OBJECTS)
	@echo "  [LD]  $@"
	@gcc $(CFLAGS) $(INCLUDE) $(LIBS) $(OBJECTS) $@.o -o $@

clean:
	rm -f $(TARGETS) $(OBJECTS) $(TARGET_OBJECTS)
