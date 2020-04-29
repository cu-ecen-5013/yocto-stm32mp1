# Recipe created by recipetool
# This is the basis of a recipe and may need further editing in order to be fully functional.
# (Feel free to remove these comments when editing.)

# WARNING: the following LICENSE and LIC_FILES_CHKSUM values are best guesses - it is
# your responsibility to verify that the values are complete and correct.
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://git@github.com/cu-ecen-5013/ecen5013-hello-world.git;protocol=ssh"

# Modify these as desired
PV = "1.0+git${SRCPV}"
SRCREV = "452e6e18d1323df20a309e1e55300063d893777e"

S = "${WORKDIR}/git"

FILES_PN += "\
			${bindir_native}/hello_world \
			"

do_configure () {
	# Specify any needed configure commands here
	:
}

do_compile () {
	# Specify compilation commands here
	oe_runmake
}

do_install () {
	# Specify install commands here
	install -d ${D}${bindir_native}
	install -m 0755 ${S}/hello-ecen5013 ${D}${bindir_native}
}
