PN = "tuxbox-links"
PV = "${DISTRO_VERSION}"
PR = "r7"

SRC_URI = "file://tuxbox-links.sh"

INITSCRIPT_NAME = "tuxbox-links"
INITSCRIPT_PARAMS = "start 07 S ."

inherit update-rc.d

do_install() {
	install -d ${D}/etc/init.d
	install -m 0755 ${WORKDIR}/tuxbox-links.sh ${D}/etc/init.d/tuxbox-links
}

# with previous versions, we created a /etc/tuxbox/config directory, instead of a link.
# Remove it, because we cannot overwrite a directory with a link
pkg_postinst_${PN} () {
	[ -d /etc/tuxbox/config ] && rm -Rf /etc/tuxbox/config
	/etc/init.d/tuxbox-links
}
