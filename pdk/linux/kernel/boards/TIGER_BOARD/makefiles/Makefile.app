#
#
## -= Makefile for customer userspace application =-
#
#

CSTM_APP = tools/test_bench

all:
	rm -f $(SRC_PF_SUBDIR)/cstm.def
	@if [ -f $(BOARD_DIR)/makefiles/cstm.def ]; then \
	  @echo "Copy cstm.def to platform/src/ before compile..."; \
	  cp -raf $(BOARD_DIR)/makefiles/cstm.def $(SRC_PF_SUBDIR)/; \
	else \
	  echo "File cstm.def does not exist!!"; \
	fi

	@echo "Begin to build $(CSTM_APP)...\n"; \
	cd $(PLATFORM_DIR)/$(CSTM_APP); \
	$(MAKE) -C $(PLATFORM_DIR)/$(CSTM_APP)

app:
	rm -f $(SRC_PF_SUBDIR)/cstm.def
	@if [ -f $(BOARD_DIR)/makefiles/cstm.def ]; then \
	  @echo "Copy cstm.def to platform/src/ before compile..."; \
	  cp -raf $(BOARD_DIR)/makefiles/cstm.def $(SRC_PF_SUBDIR)/; \
	else \
	  echo "File cstm.def does not exist!!"; \
	fi

	@echo "Begin to build $(CSTM_APP)...\n"; \
	cd $(PLATFORM_DIR)/$(CSTM_APP); \
	$(MAKE) -C $(PLATFORM_DIR)/$(CSTM_APP) all

lib:
	rm -f $(SRC_PF_SUBDIR)/cstm.def
	@if [ -f $(BOARD_DIR)/makefiles/cstm.def ]; then \
	  @echo "Copy cstm.def to platform/src/ before compile..."; \
	  cp -raf $(BOARD_DIR)/makefiles/cstm.def $(SRC_PF_SUBDIR)/; \
	else \
	  echo "File cstm.def does not exist!!"; \
	fi

	@echo "Begin to build $(CSTM_APP)...\n"; \
	cd $(PLATFORM_DIR)/$(CSTM_APP); \
	$(MAKE) -C $(PLATFORM_DIR)/$(CSTM_APP) release

clean:
	rm -f $(SRC_PF_SUBDIR)/cstm.def
	@if [ -f $(BOARD_DIR)/makefiles/Makefile.src ]; then \
	  @echo "Copy Mafile.src to platform/src/ before compile..."; \
	  cp -raf $(BOARD_DIR)/makefiles/Makefile.src $(SRC_PF_SUBDIR)/Makefile; \
	fi

	@echo "Begin to build $(CSTM_APP)...\n"; \
	cd $(PLATFORM_DIR)/$(CSTM_APP); \
	$(MAKE) -C $(PLATFORM_DIR)/$(CSTM_APP) clean

release:
	rm -f $(SRC_PF_SUBDIR)/cstm.def
	@if [ -f $(BOARD_DIR)/makefiles/Makefile.src ]; then \
	  @echo "Copy Mafile.src to platform/src/ before compile..."; \
	  cp -raf $(BOARD_DIR)/makefiles/Makefile.src $(SRC_PF_SUBDIR)/Makefile; \
	fi

	@echo "Begin to build $(CSTM_APP)...\n"; \
	cd $(PLATFORM_DIR)/$(CSTM_APP); \
	$(MAKE) -C $(PLATFORM_DIR)/$(CSTM_APP) ddk_rel

