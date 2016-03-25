
DIRS = mali
CLEAN_DIRS =

.PHONY : $(DIRS)

all: $(DIRS)


mali:
	mkdir -p $(IMAGES_DIR)/lib/mali
	cd $(SRC_DIR)/thirddrivers/mali/Mali_Linux/; tar -zxv -f ./lib_release.tar; 
	cp -rfp mali/Mali_Linux/* $(IMAGES_DIR)/lib/mali/; \
	mkdir -p $(IMAGES_DIR)/include/thirddrivers/mali
	cp -rfp mali/include/* $(IMAGES_DIR)/include/thirddrivers/mali/; \

clean: 
	for dir in $(CLEAN_DIRS); do \
        $(MAKE) -C $$dir clean; \
	done

