# host tools should not use cross compiler
unexport AR AS LD CC CPP CXX NM STRIP SSTRIP OBJCOPY OBJDUMP RANLIB

CC		= gcc
STRIP	= strip
export CC STRIP

CFLAGS_NOARCH	= -O2 -DGNU
CFLAGS			= $(CFLAGS_NOARCH) -m32
export CFLAGS


.PHONY: 

all: 
	@echo "Nothing to do."


# Cleaning rules
clean:
	@echo "Clean hosttools."
	rm -f scripts/lxdialog/*.o
	rm -f scripts/lxdialog/lxdialog

