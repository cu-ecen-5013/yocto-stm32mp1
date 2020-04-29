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

if grep -q "virtualization" ./conf/local.conf; then
    echo -n ""
else
    echo "DISTRO_FEATURES_append=\" virtualization\"" >> ./conf/local.conf
fi

if grep -q "PACKAGECONFIG_pn-azure-iot-sdk-c_append" ./conf/local.conf; then
    echo -n ""
else
    echo "PACKAGECONFIG_pn-azure-iot-sdk-c_append = \"amqp mqtt uhttp edge\"" >> ./conf/local.conf
fi

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


bitbake-layers show-layers | grep "meta-filesystems" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	echo "Adding meta-filesystems layer"
	bitbake-layers add-layer ../layers/meta-openembedded/meta-filesystems
else
	echo "meta-filesystems layer already exists"
fi

bitbake-layers show-layers | grep "meta-virtualization" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	echo "Adding meta-virtualization layer"
	bitbake-layers add-layer ../layers/meta-virtualization
else
	echo "meta-virtualization layer already exists"
fi

bitbake-layers show-layers | grep "meta-rust" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	echo "Adding meta-rust layer"
	bitbake-layers add-layer ../layers/meta-rust
else
	echo "meta-rust layer already exists"
fi

bitbake-layers show-layers | grep "meta-iotedge" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	echo "Adding meta-iotedge layer"
	bitbake-layers add-layer ../layers/meta-iotedge
else
	echo "meta-iotedge layer already exists"
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
bitbake st-image-aesd