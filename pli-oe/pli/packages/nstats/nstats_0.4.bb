SECTION = "console/utils"
DESCRIPTION = "nstats is a collection of utilities to monitor and analyze your network"
LICENSE = "GPL"

SRC_URI = "http://trash.net/~reeler/nstats/files/stable/source/nstats-${PV}.tar.gz"

S = "${WORKDIR}/nstats-${PV}"

inherit autotools
