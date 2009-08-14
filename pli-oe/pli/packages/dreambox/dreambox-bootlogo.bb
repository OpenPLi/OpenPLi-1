DESCRIPTION = "Bootlogo support"
SECTION = "base"
PRIORITY = "required"
LICENSE = "proprietary"
MAINTAINER = "Felix Domke <tmbinc@elitedvb.net>"

IMAGES_VERSION = "1"
BINARY_VERSION = "1"
BINARY_VERSION_dm7025 = "2"
BINARY_VERSION_dm800 = "2"
BINARY_VERSION_dm8000 = "6"

PV = "${DISTRO_VERSION}"
PR = "r13"

SRC_URI = "http://sources.dreamboxupdate.com/download/7020/bootlogo-${MACHINE}-${BINARY_VERSION}.elf \
	http://sources.dreamboxupdate.com/download/7020/bootlogo-${MACHINE}-${IMAGES_VERSION}.mvi \
	http://sources.dreamboxupdate.com/download/7020/bootlogo_wait-${MACHINE}-${IMAGES_VERSION}.mvi \
	http://sources.dreamboxupdate.com/download/7020/backdrop-${MACHINE}-${IMAGES_VERSION}.mvi"

SRC_URI_append_dm8000 = " http://sources.dreamboxupdate.com/download/7020/bootlogo-${MACHINE}-${IMAGES_VERSION}.jpg"

SRC_URI_append_dm800 = " http://sources.dreamboxupdate.com/download/7020/bootlogo-${MACHINE}-${IMAGES_VERSION}.jpg \
	http://sources.dreamboxupdate.com/download/7020/switchoff-${MACHINE}-${IMAGES_VERSION}.mvi"

S = "${WORKDIR}/"

###
###PLi stuff begin
SRC_URI += " file://bootlogo.mvi \
	file://bootlogo_wait.mvi \
	file://backdrop.mvi"

SRC_URI_append += " file://bootlogo.jpg"

do_pli_preinstall() {
	install -m 0755 ${S}/bootlogo.mvi ${S}/bootlogo-${MACHINE}-${IMAGES_VERSION}.mvi
	install -m 0755 ${S}/bootlogo_wait.mvi ${S}/bootlogo_wait-${MACHINE}-${IMAGES_VERSION}.mvi
	install -m 0755 ${S}/backdrop.mvi ${S}/backdrop-${MACHINE}-${IMAGES_VERSION}.mvi
	install -m 0755 ${S}/bootlogo.jpg ${S}/bootlogo-${MACHINE}-${IMAGES_VERSION}.jpg
}

addtask pli_preinstall after do_compile before do_install
###PLi stuff end
###

MVI = "bootlogo backdrop bootlogo_wait"
MVI_append_dm800 = " switchoff"

do_install() {
	install -d ${D}/boot
	install -m 0755 ${S}/bootlogo-${MACHINE}-${BINARY_VERSION}.elf ${D}/boot/bootlogo.elf
	for i in ${MVI}; do
		install -m 0755 ${S}/$i-${MACHINE}-${IMAGES_VERSION}.mvi ${D}/boot/$i.mvi;
	done;
}

do_install_dm800() {
	install -d ${D}/boot
	install -d ${D}/usr/share
	install -m 0755 ${S}/bootlogo-${MACHINE}-${BINARY_VERSION}.elf ${D}/boot/bootlogo.elf
	install -m 0755 ${S}/bootlogo-${MACHINE}-${IMAGES_VERSION}.jpg ${D}/boot/bootlogo.jpg
	for i in ${MVI}; do
		install -m 0755 ${S}/$i-${MACHINE}-${IMAGES_VERSION}.mvi ${D}/usr/share/$i.mvi;
		ln -sf /usr/share/$i.mvi ${D}/boot/$i.mvi;
	done;
}

do_install_dm8000() {
	do_install_dm800
}

pkg_preinst() {
	[ -d /proc/stb ] && mount -o rw,remount /boot
	true
}

pkg_postinst() {
	[ -d /proc/stb ] && mount -o ro,remount /boot
	true
}

pkg_prerm() {
	[ -d /proc/stb ] && mount -o rw,remount /boot
	true
}

pkg_postrm() {
	[ -d /proc/stb ] && mount -o ro,remount /boot
	true
}

PACKAGE_ARCH := "${MACHINE_ARCH}"
FILES_${PN} = "/boot /usr/share"
