#! /bin/bash
# Script to build STM32MP1 Yocto Configuration
# Author - Atharva Nandanwar

parent_dir=$(dirname $0)

# Initialize all git modules
git submodule init
# Sync them
git submodule update --init --recursive
# Source the build environment
# TODO: Change the machine name and distribution if you are making changes to that.
DISTRO=openstlinux-weston MACHINE=stm32mp1 source ./layers/meta-st/scripts/envsetup.sh

bitbake-layers show-layers | grep "meta-oe" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	echo "Adding meta-oe layer"
	bitbake-layers add-layer ../layers/meta-openembedded/meta-oe
else
	echo "meta-oe layer already exists"
fi

bitbake-layers show-layers | grep "meta-python" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	echo "Adding meta-python layer"
	bitbake-layers add-layer ../layers/meta-openembedded/meta-python
else
	echo "meta-python layer already exists"
fi

# TODO: Add more layers in the similar format

bitbake-layers show-layers | grep "meta-aesd" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	echo "Adding meta-aesd layer"
	bitbake-layers add-layer ../layers/meta-st/meta-aesd
else
	echo "meta-aesd layer already exists"
fi

set -e

# TODO: Change st-image-weston with the image you want to build
bitbake st-image-weston