#!bin/bash

export PATH=/home/zengjianming/compliation_tool/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin:$PATH

make CROSS_COMPILE=arm-linux-gnueabihf- clean -j12
make CROSS_COMPILE=arm-linux-gnueabihf- distclean -j12

# make CROSS_COMPILE=arm-linux-gnueabihf- imx6ul_zimin_board_sd_defconfig -j12
# make CROSS_COMPILE=arm-linux-gnueabihf- imx6ul-zimin-board-sd.dtb -j12
# make CROSS_COMPILE=arm-linux-gnueabihf- zImage -j12

# sudo mount /dev/sda1 /mnt/imx6ul-ws/boot
# sudo mount /dev/sda2 /mnt/imx6ul-ws/rootfs

# make CROSS_COMPILE=arm-linux-gnueabihf- modules -j4 
# sudo make CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=/mnt/imx6ul-ws/rootfs modules_install

# sudo cp arch/arm/boot/zImage /mnt/imx6ul-ws/boot/zImage
# sudo cp arch/arm/boot/dts/imx6ul-zimin-board-sd.dtb /mnt/imx6ul-ws/boot/imx6ul-zimin-board-sd.dtb

# sudo sync

# sudo umount /mnt/imx6ul-ws/boot
# sudo umount /mnt/imx6ul-ws/rootfs