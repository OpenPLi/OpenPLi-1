DESCRIPTION = "screen capture tool"
MAINTAINER = "Sven Karschewski <seddi@i-have-a-dreambox.com>"

PN = "grab"
PV = "0.6"
PR = "r0"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/external;module=${PN};rev=${PLISVNREV};proto=${PLISVNPROTO}"

S = "${WORKDIR}/${PN}"

inherit autotools

FILES_${PN} = "/"
