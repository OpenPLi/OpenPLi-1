PN = "plimgr"
PV = "${DISTRO_VERSION}"
PR = "r1"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/cdk/cdk;module=allboxes;rev=${PLISVNREV};proto=${PLISVNPROTO} \
	file://plimgr.sh"

S = "${WORKDIR}/plimgr"

INITSCRIPT_NAME = "plimgr"
INITSCRIPT_PARAMS = "defaults 40 20"

inherit autotools pkgconfig update-rc.d

#TODO: find a way to avoid duplicating these install steps

#for these boxes plimgr itself is installed with pli-plugins
do_install_dm600pvr() {
	install -d ${D}/etc/init.d
	install -m 0755 ${WORKDIR}/plimgr.sh ${D}/etc/init.d/plimgr
	install -d ${D}/etc/plimgr/cams
	install -d ${D}/etc/plimgr/cardservers
	install -d ${D}/etc/plimgr/services
	install -d ${D}/etc/plimgr/scripts
	install -m 0755 `find ${WORKDIR}/allboxes/var_init/etc/plimgr/scripts -maxdepth 1 -type f` ${D}/etc/plimgr/scripts
}

do_install_dm500plus() {
	install -d ${D}/etc/init.d
	install -m 0755 ${WORKDIR}/plimgr.sh ${D}/etc/init.d/plimgr
	install -d ${D}/etc/plimgr/cams
	install -d ${D}/etc/plimgr/cardservers
	install -d ${D}/etc/plimgr/services
	install -d ${D}/etc/plimgr/scripts
	install -m 0755 `find ${WORKDIR}/allboxes/var_init/etc/plimgr/scripts -maxdepth 1 -type f` ${D}/etc/plimgr/scripts
}

do_install_dm7020() {
	install -d ${D}/etc/init.d
	install -m 0755 ${WORKDIR}/plimgr.sh ${D}/etc/init.d/plimgr
	install -d ${D}/etc/plimgr/cams
	install -d ${D}/etc/plimgr/cardservers
	install -d ${D}/etc/plimgr/services
	install -d ${D}/etc/plimgr/scripts
	install -m 0755 `find ${WORKDIR}/allboxes/var_init/etc/plimgr/scripts -maxdepth 1 -type f` ${D}/etc/plimgr/scripts
}

do_install_append() {
	install -d ${D}/etc/init.d
	install -m 0755 ${WORKDIR}/plimgr.sh ${D}/etc/init.d/plimgr
	install -d ${D}/etc/plimgr/cams
	install -d ${D}/etc/plimgr/cardservers
	install -d ${D}/etc/plimgr/services
	install -d ${D}/etc/plimgr/scripts
	install -m 0755 `find ${WORKDIR}/allboxes/var_init/etc/plimgr/scripts -maxdepth 1 -type f` ${D}/etc/plimgr/scripts
	rm ${D}/usr/bin/pli_pmthelper
}
