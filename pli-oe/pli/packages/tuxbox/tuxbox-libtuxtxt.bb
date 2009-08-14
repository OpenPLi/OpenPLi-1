DESCRIPTION = "tuxbox libtuxtxt"
DEPENDS = "dreambox-dvbincludes"
MAINTAINER = "Felix Domke <tmbinc@elitdvb.net>"

SRCDATE = "20090130"

PN = "libtuxtxt"
PV = "0.0+cvs${SRCDATE}"
PR = "r0"

SRC_URI = "cvs://anoncvs@cvs.tuxbox.org/cvs/tuxbox;module=apps/tuxbox/libs;method=ext \
	file://acinclude.m4 \
	file://ignorelibs.patch;patch=1 \
	file://nolibtuxbox.diff;patch=1"

SRC_URI_append_dm7025 = " \
	file://32bpp.diff;patch=1 \
	file://resize_framebuffer.diff;patch=1 \
	file://allow_different_demux.diff;patch=1 \
	file://fastclear.diff;patch=1"

SRC_URI_append_dm800 = " \
	file://32bpp.diff;patch=1 \
	file://resize_framebuffer.diff;patch=1 \
	file://allow_different_demux.diff;patch=1 \
	file://fastclear.diff;patch=1"

SRC_URI_append_dm8000 = " \
	file://32bpp.diff;patch=1 \
	file://resize_framebuffer.diff;patch=1 \
	file://allow_different_demux.diff;patch=1 \
	file://fastclear.diff;patch=1"

S = "${WORKDIR}/libs"
EXTRA_OECONF = "--with-target=native"

CFLAGS_append = " -DHAVE_DREAMBOX_HARDWARE -DDREAMBOX"

inherit autotools pkgconfig

FILES_libtuxtxt_append = " /usr/lib/libtuxtxt.so"

FILES_${PN}-dev = "/usr/lib/libtuxtxt.la /usr/lib/pkgconfig/tuxbox-tuxtxt.pc"

python populate_packages_prepend () {
	tuxbox_libdir = bb.data.expand('${libdir}', d)
}

do_configure_prepend() {
	install ${WORKDIR}/acinclude.m4 ${S}/acinclude.m4
}

do_stage() {
	install -m 0644 ${S}/libtuxtxt/tuxtxt_*.h ${STAGING_INCDIR}
	install -d ${STAGING_INCDIR}/tuxtxt
	install -m 0644 ${S}/libtuxtxt/tuxtxt_*.h ${STAGING_INCDIR}/tuxtxt
	cd libtuxtxt
	oe_libinstall -so libtuxtxt ${STAGING_LIBDIR}
	cd ..
}

do_install_prepend() {
	cd libtuxtxt
}
