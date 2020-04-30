# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# WARNING: the following LICENSE and LIC_FILES_CHKSUM values are best guesses - it is
# your responsibility to verify that the values are complete and correct.
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/azure-sample-application/LICENSE;md5=e5196924dff41bc1b1f5c4670a4fed65"

SRC_URI = "git://git@github.com/cu-ecen-5013/yocto-stm32mp1;protocol=ssh;branch=azure-iot \
		   "

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "6cb0d802769c10981f48a77cd659cde4a51c3f8d"

S = "${WORKDIR}/git/azure-sample-application"
B = "${WORKDIR}/build"

FILES_${PN} = "\
    ${bindir}/* \
"

DEPENDS_append += " \
	azure-c-shared-utility \
	azure-iot-sdk-c \
	"

RDEPENDS_${PN} += " \
	iotedge-cli \
	iotedge-daemon \
	"

inherit cmake

do_install() {
	# Specify install commands here
	install -d ${D}${bindir_native}
	install -m 0755 ${B}/azure_app ${D}${bindir_native}
}

