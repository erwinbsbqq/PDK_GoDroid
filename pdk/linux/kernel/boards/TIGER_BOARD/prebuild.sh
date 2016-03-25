#!/bin/sh

# use shell funcs
. $BUILD_DIR/funcs.sh


# copy kernel patch
copy $BOARD_DIR/kernel_patch/drivers/base/Kconfig $LINUX_DIR/drivers/base/Kconfig -f
copy $BOARD_DIR/kernel_patch/drivers/base/Makefile $LINUX_DIR/drivers/base/Makefile -f
copy $BOARD_DIR/kernel_patch/drivers/base/sync.c $LINUX_DIR/drivers/base/sync.c -f
copy $BOARD_DIR/kernel_patch/include/linux/sync.h $LINUX_DIR/include/linux/sync.h -f
copy $BOARD_DIR/kernel_patch/include/trace/events/sync.h $LINUX_DIR/include/trace/events/sync.h -f
copy $BOARD_DIR/kernel_patch/net/wireless/Kconfig $LINUX_DIR/net/wireless/Kconfig -f
copy $BOARD_DIR/kernel_patch/aliir/ali_ir_g2.c $ALIDRIVERS_KERNEL_SUBDIR/aliir/ali_ir_g2.c -f
copy $BOARD_DIR/kernel_patch/drivers/input/misc/mpu6050.c $LINUX_DIR/drivers/input/misc/mpu6050.c -f
copy $BOARD_DIR/kernel_patch/drivers/input/misc/max31855.c $LINUX_DIR/drivers/input/misc/max31855.c -f
mkdir $LINUX_DIR/drivers/sensors
copy $BOARD_DIR/kernel_patch/drivers/sensors/sensors_class.c $LINUX_DIR/drivers/sensors/sensors_class.c -f
copy $BOARD_DIR/kernel_patch/drivers/sensors/sensors_ssc.c $LINUX_DIR/drivers/sensors/sensors_ssc.c -f
copy $BOARD_DIR/kernel_patch/drivers/sensors/Kconfig $LINUX_DIR/drivers/sensors/Kconfig -f
copy $BOARD_DIR/kernel_patch/drivers/sensors/Makefile $LINUX_DIR/drivers/sensors/Makefile -f
copy $BOARD_DIR/kernel_patch/include/linux/input/mpu6050.h $LINUX_DIR/include/linux/input/mpu6050.h -f
copy $BOARD_DIR/kernel_patch/include/linux/sensors.h $LINUX_DIR/include/linux/sensors.h -f
copy $BOARD_DIR/kernel_patch/include/linux/input.h $LINUX_DIR/include/linux/input.h -f
copy $BOARD_DIR/kernel_patch/drivers/Makefile $LINUX_DIR/drivers/Makefile -f
copy $BOARD_DIR/kernel_patch/drivers/Kconfig $LINUX_DIR/drivers/Kconfig -f
copy $BOARD_DIR/kernel_patch/drivers/input/misc/Makefile $LINUX_DIR/drivers/input/misc/Makefile -f
copy $BOARD_DIR/kernel_patch/drivers/input/misc/Kconfig $LINUX_DIR/drivers/input/misc/Kconfig -f
copy $BOARD_DIR/kernel_patch/drivers/tty/serial/ali_uart.c $LINUX_DIR/drivers/tty/serial/ali_uart.c -f
copy $BOARD_DIR/kernel_patch/alidrivers/modules/alipmu/ali_pmu_bin_3921.h $BOARD_DIR/../../alidrivers/modules/alipmu/ali_pmu_bin_3921.h -f
copy $BOARD_DIR/kernel_patch/alidrivers/modules/alimmc/host/ali_mci_bsc.h $BOARD_DIR/../../alidrivers/modules/alimmc/host/ali_mci_bsc.h -f
$BOARD_DIR/flashfiles/genrawpart.sh
