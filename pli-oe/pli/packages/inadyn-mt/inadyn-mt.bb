PV = "v.02.12.20"
PR = "r1"

SRC_URI = "http://heanet.dl.sourceforge.net/sourceforge/inadyn-mt/inadyn-mt.${PV}.tar.gz \
	file://inadyn-mt.sh"

S = "${WORKDIR}/inadyn-mt"

inherit update-rc.d

INITSCRIPT_NAME = "inadyn-mt"

do_compile() {
	make -f makefile
}

do_install() {
	install -d ${D}/usr/bin
	${STRIP} ${S}/bin/linux/inadyn-mt
	install -m 755 ${S}/bin/linux/inadyn-mt ${D}/usr/bin
	install -d ${D}/etc/init.d
	install -m 755 ${WORKDIR}/inadyn-mt.sh ${D}/etc/init.d/inadyn-mt
}
