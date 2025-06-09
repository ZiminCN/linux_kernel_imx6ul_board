#!bin/bash

export PATH=/home/zengjianming/compliation_tool/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin:$PATH

# make CROSS_COMPILE=arm-linux-gnueabihf- clean -j12
make CROSS_COMPILE=arm-linux-gnueabihf- distclean -j12

# make CROSS_COMPILE=arm-linux-gnueabihf- imx6ul_zimin_board_sd_defconfig -j12
# make CROSS_COMPILE=arm-linux-gnueabihf- imx6ul-zimin-board-sd.dtb -j12
# make CROSS_COMPILE=arm-linux-gnueabihf- zImage -j12

# make CROSS_COMPILE=arm-linux-gnueabihf- modules -j4
