export CUR_DIR = $(shell pwd)
export PDKAPI_DIR = $(CUR_DIR)
export PDKAPI_INSTALL = $(PDKAPI_DIR)/install

include $(CUR_DIR)/compiler

############################################################################
#
# PHONY targets
#
############################################################################
.PHONY: all

all:
	echo "Will build pdkapi......"; \
	echo "CROSS_COMPILE:$(CROSS_COMPILE)"; \
	echo "COMMON_CFLAGS:$(COMMON_CFLAGS)"; \
	rm -rf $(PDKAPI_INSTALL)/*; \
	mkdir -p $(PDKAPI_INSTALL); \
	mkdir -p $(PDKAPI_INSTALL)/bin; \
	mkdir -p $(PDKAPI_INSTALL)/obj; \
	mkdir -p $(PDKAPI_INSTALL)/rel; \
	mkdir -p $(PDKAPI_INSTALL)/lib; \
	mkdir -p $(PDKAPI_INSTALL)/include; \
	mkdir -p $(PDKAPI_INSTALL)/etc; \
	cd $(PDKAPI_DIR)/src/hld/build/; \
	$(MAKE) all || exit "$$?";
	echo "build pdkapi finished!"; \

clean:
	echo "Will clean pdkapi......";	\
	cd $(PDKAPI_DIR)/src/hld/build/; \
	$(MAKE) clean || exit "$$?"; \
	rm -rf $(PDKAPI_INSTALL)/* ; \
	echo "clean pdkapi finished!"; \
