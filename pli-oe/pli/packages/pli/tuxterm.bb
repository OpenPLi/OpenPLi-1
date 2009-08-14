DESCRIPTION = "TuxTerm"
MAINTAINER = "Sven Karschewski <seddi@i-have-a-dreambox.com>"
DEPENDS = "freetype"

PN = "tuxterm"
PV = "0.2"
PR = "r1"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/external;module=${PN};rev=${PLISVNREV};proto=${PLISVNPROTO}"

S = "${WORKDIR}/${PN}"

inherit autotools

FILES_${PN} = "/"

EXTRA_OECONF = "--with-bpp=8"
