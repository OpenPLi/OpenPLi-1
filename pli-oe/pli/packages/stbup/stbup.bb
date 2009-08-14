PN = "stbup"
PV = "0.1.7"
PR = "r1"

SRC_URI = "file://stbup.c file://stbup.sh file://stbup.conf"

S = "${WORKDIR}"

INITSCRIPT_NAME = "stbup"
INITSCRIPT_PARAMS = "defaults 40 20"

inherit update-rc.d

do_compile() {
	${CC} stbup.c -o stbup
}

do_install() {
	install -d ${D}/usr/bin
	install -m 0755 ${S}/stbup ${D}/usr/bin/
	install -d ${D}/etc/init.d
	install -m 0755 ${S}/stbup.sh ${D}/etc/init.d/stbup
	install -d ${D}/etc
	install -m 0644 ${S}/stbup.conf ${D}/etc/stbup.conf.example
}

pkg_postinst_${PN} () {
	[ -e $D/etc/stbup.conf ] || cp $D/etc/stbup.conf.example $D/etc/stbup.conf
}