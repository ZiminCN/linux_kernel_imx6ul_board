#!bin/bash

make CROSS_COMPILE=arm-linux-gnueabihf- clean
# make CROSS_COMPILE=arm-linux-gnueabihf- distclean
make CROSS_COMPILE=arm-linux-gnueabihf- imx6ul_board_defconfig
make CROSS_COMPILE=arm-linux-gnueabihf- imx6ul-board-nand.dtb
make CROSS_COMPILE=arm-linux-gnueabihf- zImage -j4
# make CROSS_COMPILE=arm-linux-gnueabihf- modules -j4
