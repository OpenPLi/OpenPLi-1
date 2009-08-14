DESCRIPTION = "Common files for PLi image"
LICENSE = "GPL"
MAINTAINER = "PLi team"
RDEPENDS = "enigma"

#we add these packages to our DEPENDS in order to build them, the user can optionally install them
DEPENDS += "pli-optional-packages"

PN = "pli-files"
PV = "${DISTRO_VERSION}"
PR = "r2"

DREAMVERSION = "0109"

FILESPATH = "${@base_set_filespath([ '${FILE_DIRNAME}/pli-files/${P}', '${FILE_DIRNAME}/pli-files', '${FILE_DIRNAME}' ], d)}"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/cdk/cdk;module=allboxes;rev=${PLISVNREV};proto=${PLISVNPROTO} \
	${PLISVNURL}/${PLISVNBRANCH}/cdk/cdk/standard;module=${MACHINE};rev=${PLISVNREV};proto=${PLISVNPROTO} \
	file://gsub.sh \
	file://misc_services.sh"

SRC_URI_append_dm7020 = " file://lcdimage.sh"

FILES_${PN} = "/"

S = "${WORKDIR}/allboxes"

inherit update-rc.d
INITSCRIPT_NAME = "misc_services"
INITSCRIPT_PARAMS = "start 30 2 3 ."

do_install() {
	install -d ${D}/etc/init.d
	install -d ${D}/etc/ppanels
	install -d ${D}/etc/tuxbox/installer
	install -d ${D}/etc/tuxbox/scce
	install -d ${D}/usr/bin
	install -d ${D}/usr/keys
	install -d ${D}/usr/lib/tuxbox/plugins
	install -d ${D}/usr/scam
	install -d ${D}/usr/share/misc
	install -d ${D}/usr/share/pli-files/default/etc
	install -d ${D}/usr/share/tuxbox
	install -d ${D}/media/server1
	install -d ${D}/media/server2
	install -d ${D}/media/server3
	install -d ${D}/media/server4
	install -d ${D}/media/server5
	install -m 0755 ${WORKDIR}/${MACHINE}/bin/* ${D}/usr/bin
	install -m 0755 ${S}/bin/cardinfo-pli.sh ${D}/usr/bin
	install -m 0755 ${S}/bin/install.sh ${D}/usr/bin
	install -m 0755 ${S}/bin/ppanelupdate.sh ${D}/usr/bin
	install -m 0755 ${S}/var_init/bin/* ${D}/usr/bin
	install -m 0644 `find ${S}/var_init/etc -maxdepth 1 -type f` ${D}/usr/share/pli-files/default/etc
	install -m 0644 `find ${S}/var_init/etc/ppanels -maxdepth 1 -type f` ${D}/etc/ppanels
	install -m 0644 "`find ${S}/var_init/tuxbox/installer -maxdepth 1 -type f`" ${D}/etc/tuxbox/installer
	if [ ${MACHINE} != "dm600pvr" -a ${MACHINE} != "dm500plus" ]; then
		install -m 0755 ${WORKDIR}/lcdimage.sh ${D}/etc/init.d/lcdimage
		install -d ${D}/etc/rcS.d
		[ -e ${D}/etc/rcS.d/S40lcdimage ] && rm ${D}/etc/rcS.d/S40lcdimage
		ln -s ../init.d/lcdimage ${D}/etc/rcS.d/S40lcdimage
	fi
	install -m 0644 ${WORKDIR}/${MACHINE}/etc/* ${D}/etc
	install -m 0755 ${WORKDIR}/misc_services.sh ${D}/etc/init.d/misc_services
	install -m 0755 ${WORKDIR}/gsub.sh ${D}/etc/init.d/gsub
	install -m 0644 ${WORKDIR}/${MACHINE}/usr/share/misc/* ${D}/usr/share/misc
	
	#install a compatibility symlink for binaries compiled with the cdk
	ln -s libtuxbox-xmltree.so.0.0.0 ${D}/usr/lib/libxmltree.so.0
}

pkg_postinst_${PN} () {
	for fl in `find $D/usr/share/pli-files/default/etc -type f`; do \
		[ -e $D/etc/`basename $fl` ] || cp $fl $D/etc; \
	done
}
