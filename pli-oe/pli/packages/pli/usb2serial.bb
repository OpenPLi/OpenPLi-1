DESCRIPTION = "USB2serial drivers"
RDEPENDS = "kernel-module-usbserial kernel-module-ftdi-sio kernel-module-pl2303 kernel-module-belkin-sa kernel-module-keyspan"

SRC_URI = "file://usb2serial.sh"

PV = "${KERNEL_VERSION}"
PR = "r1"

S = "${WORKDIR}"

INITSCRIPT_NAME = "usb2serial"
INITSCRIPT_PARAMS = "defaults 39 20"

inherit update-rc.d

do_install() {
	install -d ${D}/etc/init.d
	install -m 0755 ${WORKDIR}/usb2serial.sh ${D}/etc/init.d/usb2serial
}