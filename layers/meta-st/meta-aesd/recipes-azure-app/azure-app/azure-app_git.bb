# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# WARNING: the following LICENSE and LIC_FILES_CHKSUM values are best guesses - it is
# your responsibility to verify that the values are complete and correct.
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${WORKDIR}/git/LICENSE;md5=96acd7c558849eaf93c9c522f1a2f334"

SRC_URI = "git://git@github.com/cu-ecen-5013/yocto-stm32mp1;protocol=ssh;branch=azure-iot \
		   "

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "aa9f314df6c9341fe7b2afe1b7ec92dee4c02b5c"

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

