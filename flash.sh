#! /bin/bash

# Name - Atharva Nandanwar
# File - flash.sh
# Purpose - Used to flash code into STM32MP1 connected through usb

curr_dir=`dirname $0`
build_dir="build-openstlinuxweston-stm32mp1-aesd-project"
image_dir="tmp-glibc/deploy/images/stm32mp1-aesd-project"
flash_parent_dir="flashlayout_st-image-aesd"
flash_layout_file="FlashLayout_sdcard_stm32mp157a-aesd-project-mx-trusted.tsv"
stm_flasher="${curr_dir}/STM32CubeProgrammer-2.4.0/bin/STM32_Programmer_CLI"

# Check if the driver and rules exist
# Check if necessary drivers are installed
dpkg -S libusb-1.0-0 > /dev/null
if [[ $? -ne 0 ]]; then
    echo "Installing libusb package"
    apt-get install libusb-1.0-0
    if [[ $? -ne 0 ]]; then
        echo "Please run it with sudo"
    fi
else
    echo "Package - libusb already installed"
fi

if [[ -e "/etc/udev/rules.d/50-usb-conf.rules" ]]; then
    echo "USB Driver Rules exists"
else
    cp ${curr_dir}/STM32CubeProgrammer-2.4.0/Drivers/rules/50-usb-conf.rules /etc/udev/rules.d/
fi

# Start Programming

if [ -f ${curr_dir}/${build_dir}/${image_dir}/${flash_parent_dir}/${flash_layout_file} ]; then
    echo "Commencing flash procedure"
    cp ${curr_dir}/${build_dir}/${image_dir}/${flash_parent_dir}/${flash_layout_file} ${curr_dir}/${build_dir}/${image_dir}
    ${stm_flasher} -c port=usb1 -w ${curr_dir}/${build_dir}/${image_dir}/${flash_layout_file}
else
    echo "No image files found"
fi