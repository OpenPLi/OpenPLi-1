DEPENDS = "enigma"
DESCRIPTION = "tuxbox plugins (enigma)"
MAINTAINER = "Felix Domke <tmbinc@elitdvb.net>"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/cdk/apps/tuxbox;module=plugins;rev=${PLISVNREV};proto=${PLISVNPROTO} \
	   file://disable_nonworking.diff;patch=1;pnum=1 \
	   file://fix_install_weather_pics.diff;patch=1;pnum=1"

PV = "${DISTRO_VERSION}"
PR = "r2"

CFLAGS_append = " -I${STAGING_INCDIR}/enigma -DHAVE_DREAMBOX_HARDWARE -DDREAMBOX"
CXXFLAGS_append = " -I${STAGING_INCDIR}/enigma -DHAVE_DREAMBOX_HARDWARE -DDREAMBOX"

PACKAGES = "enigma-plugin-dreamdata enigma-plugin-dbswitch \
	enigma-plugin-ngrabstart enigma-plugin-ngrabstop enigma-plugin-getset \
	enigma-plugin-movieplayer enigma-plugin-script enigma-plugin-rss \
	enigma-plugin-weather enigma-plugin-demo"

#PLi plugins
PACKAGES += "enigma-plugin-bitrate enigma-plugin-mv enigma-plugin-plimgr enigma-plugin-dbepg enigma-plugin-dreamnetcast enigma-plugin-lancontrol"
DEPENDS += "db-epg wakelan"
RDEPENDS_enigma-plugin-dbepg += "db-epg"
RDEPENDS_enigma-plugin-lancontrol += "wakelan"

FILES_enigma-plugin-dreamdata = "/usr/lib/tuxbox/plugins/dreamdata.so /usr/lib/tuxbox/plugins/dreamdata.cfg \
	/etc/tuxbox/dreamdata.xml"
FILES_enigma-plugin-dbswitch = "/usr/lib/tuxbox/plugins/dbswitch.so /usr/lib/tuxbox/plugins/dbswitch.cfg"
FILES_enigma-plugin-ngrabstart = "/usr/lib/tuxbox/plugins/ngrabstart.so /usr/lib/tuxbox/plugins/ngrabstart.cfg"
FILES_enigma-plugin-ngrabstop = "/usr/lib/tuxbox/plugins/ngrabstop.so /usr/lib/tuxbox/plugins/ngrabstop.cfg"
FILES_enigma-plugin-getset = "/usr/lib/tuxbox/plugins/enigma_getset.so /usr/lib/tuxbox/plugins/enigma_getset.cfg"
FILES_enigma-plugin-movieplayer = "/usr/lib/tuxbox/plugins/movieplayer.so /usr/lib/tuxbox/plugins/movieplayer.cfg"
FILES_enigma-plugin-script = "/usr/lib/tuxbox/plugins/script.so /usr/lib/tuxbox/plugins/script.cfg"
FILES_enigma-plugin-rss = "/usr/lib/tuxbox/plugins/rss.so /usr/lib/tuxbox/plugins/rss.cfg \
	/etc/tuxbox/feeds.xml.default"
FILES_enigma-plugin-weather = "/usr/lib/tuxbox/plugins/weather.so /usr/lib/tuxbox/plugins/weather.cfg \
	/etc/tuxbox/weather.xml /usr/share/tuxbox/weather/*.png"
FILES_enigma-plugin-demo = "/usr/lib/tuxbox/plugins/enigma_demo.so /usr/lib/tuxbox/plugins/enigma_demo.cfg"

#PLi plugin files
FILES_enigma-plugin-mv = "/usr/lib/tuxbox/plugins/extepg.so /usr/lib/tuxbox/plugins/extepg.cfg \
	/etc/tuxbox/mv/* /usr/share/tuxbox/enigma/pictures/mvicons/* /usr/bin/epgidx"
FILES_enigma-plugin-plimgr = "/usr/bin/plimgr /usr/bin/pli_pmthelper /usr/bin/pli_ecmhelper"
FILES_enigma-plugin-bitrate = "/usr/lib/tuxbox/plugins/bitrate.so /usr/lib/tuxbox/plugins/bitrate.cfg"
FILES_enigma-plugin-dbepg = "/usr/lib/tuxbox/plugins/dbepg.so /usr/lib/tuxbox/plugins/dbepg.cfg \
	/usr/bin/*.sh /etc/tuxbox/dbepg/*"
FILES_enigma-plugin-dreamnetcast = "/usr/lib/tuxbox/plugins/dreamnetcast.so /usr/lib/tuxbox/plugins/dreamnetcast.cfg /etc/tuxbox/stations.xml"
FILES_enigma-plugin-lancontrol = "/usr/lib/tuxbox/plugins/lancontrol.so /usr/lib/tuxbox/plugins/lancontrol.cfg"

PACKAGES_DYNAMIC = "tuxbox-plugins-enigma-locale-*"

S = "${WORKDIR}/plugins"

inherit autotools pkgconfig

bindir = "/usr/bin"
sbindir = "/usr/sbin"

EXTRA_OECONF = "--with-target=native --with-boxtype=${MACHINE}"

pkg_postinst_enigma-plugin-dbepg() {
	[ -e $D/etc/tuxbox/dbepg/wolf_nl.dat ] || cp $D/etc/tuxbox/dbepg/wolf_nl.dat.default $D/etc/tuxbox/dbepg/wolf_nl.dat
	[ -e $D/etc/tuxbox/dbepg/assies_nl.dat ] || cp $D/etc/tuxbox/dbepg/assies_nl.dat.default $D/etc/tuxbox/dbepg/assies_nl.dat
}

pkg_postinst_enigma-plugin-rss() {
	[ -e $D/etc/tuxbox/feeds.xml ] || cp $D/etc/tuxbox/feeds.xml.default $D/etc/tuxbox/feeds.xml
}
