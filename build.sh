#! /bin/bash
# Script to build STM32MP1 Yocto Configuration
# Author - Atharva Nandanwar

parent_dir=$(dirname $0)

# Initialize all git modules
git submodule init
# Sync them
git submodule update --init --recursive
# Source the build environment
# Change the machine name and distribution if you are making changes to that.
DISTRO=openstlinux-weston MACHINE=stm32mp1 source ./layers/meta-st/scripts/envsetup.sh

echo $parent_dir