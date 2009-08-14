SRC_URI = "http://download.videolan.org/pub/${PN}/${PV}/${PN}-${PV}.tar.gz"

S = "${WORKDIR}/${PN}-${PV}"

inherit autotools
