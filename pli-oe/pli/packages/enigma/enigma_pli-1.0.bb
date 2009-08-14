DEPENDS = "curl jpeg libungif libid3tag libmad libpng libsigc++-1.2 gettext-native tuxbox-libs dreambox-dvbincludes mtd-utils freetype tuxbox-plugins sqlite3 libvorbisidec flac libfribidi"
DESCRIPTION = "Enigma is a framebuffer-based frontend for DVB functions"
MAINTAINER = "Felix Domke <tmbinc@elitedvb.net>"
LICENSE = "GPL"

PV = "${DISTRO_VERSION}"
PN = "enigma"
PR = "r1"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/cdk/apps/tuxbox;module=${PN};rev=${PLISVNREV};proto=${PLISVNPROTO} \
	file://enigma.sh \
	file://config \
	file://userbouquet.33fc5.tv \
	file://userbouquets.tv.epl \
	http://sources.dreamboxupdate.com/download/opendreambox/enigma/boot-${MACHINE} \
	file://enigma_enter_standby.sh \
	file://enigma_leave_standby.sh \
	file://rotor_fix.diff;patch=1;pnum=1 \
	file://disable_boot.diff;patch=1;pnum=1"

SRC_URI_append_dm600pvr = " http://sources.dreamboxupdate.com/download/opendreambox/enigma/showshutdownpic-${MACHINE}"
SRC_URI_append_dm500plus = " http://sources.dreamboxupdate.com/download/opendreambox/enigma/showshutdownpic-${MACHINE}"

S = "${WORKDIR}/enigma"

PACKAGES = "enigma"
PACKAGES_DYNAMIC = "enigma-locale-*"

FILES_${PN} += " ${datadir}/tuxbox ${datadir}/fonts"

inherit autotools pkgconfig

EXTRA_OECONF = "--with-target=native --with-boxtype=${MACHINE} --with-webif=expert --with-epg=private --with-reiserfs=no --with-mhw-epg --with-dish-epg"
CXXFLAGS_append = " -DHAVE_DREAMBOX_HARDWARE"

do_configure_prepend() {
	mkdir -p m4
}

do_compile_prepend() {
	chmod ugo+x ${S}/po/xml2po
}

do_stage_append() {
	install -d ${STAGING_INCDIR}/enigma
	install -m 0644 ${WORKDIR}/enigma/include/*.h ${STAGING_INCDIR}/enigma
	for dir in base dvb dvb/lowlevel codecs driver gdi gui socket system picviewer movieplayer; do
		install -d ${STAGING_INCDIR}/enigma/lib/$dir;
		install -m 0644 ${WORKDIR}/enigma/include/lib/$dir/*.h ${STAGING_INCDIR}/enigma/lib/$dir;
	done;
	rm -R ${STAGING_INCDIR}/enigma/src 2> /dev/null || /bin/true
	install -m 0644 ${WORKDIR}/enigma/src/*.h ${STAGING_INCDIR}/enigma
	ln -sf ${STAGING_INCDIR}/enigma ${STAGING_INCDIR}/enigma/src
}

do_install_append() {
	install -d ${D}/usr/share/enigma/default
	install -d ${D}/usr/share/enigma/default/cable
	install -d ${D}/usr/share/enigma/default/terrestrial
	mv ${D}/etc/enigma/* ${D}/usr/share/enigma/default 2> /dev/null || /bin/true
	rm -R ${D}/etc/enigma 2> /dev/null || /bin/true
	install -m 0644 ${WORKDIR}/config ${D}/usr/share/enigma/default/
	if [ "${MACHINE}" = "dm600pvr" -o "${MACHINE}" = "dm500plus" ]; then
		install -m 0755 ${WORKDIR}/showshutdownpic-${MACHINE} ${D}/usr/bin/showshutdownpic
		# vulcan-based boxes don't look that well with too much alpha
		echo "i:/ezap/osd/alpha=00000000" >> ${D}/usr/share/enigma/default/config
		#echo "i:/ezap/osd/simpleMainMenu=00000001" >> ${D}/usr/share/enigma/default/config
	fi
	install -m 0644 ${WORKDIR}/userbouquet* ${D}/usr/share/enigma/default/
	install -m 0755 ${WORKDIR}/enigma.sh ${D}/usr/bin/
	install -m 0755 ${WORKDIR}/boot-${MACHINE} ${D}/usr/bin/boot
	install -d ${D}/etc
	install -m 0755 ${WORKDIR}/enigma_enter_standby.sh ${D}/etc
	install -m 0755 ${WORKDIR}/enigma_leave_standby.sh ${D}/etc
}
