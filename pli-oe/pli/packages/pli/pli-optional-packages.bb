DESCRIPTION = "meta file for optional packages"
PROVIDES = "pli-optional-packages"

PR = "r3"

ENIGMADEPENDS = "openvpn gdb strace screen procps tcpdump nfs-utils ntp openssh lame ctorrent aio-grab stbup nano wakelan ushare djmount inadyn-mt"

DEPENDS_dm7020 = ${ENIGMADEPENDS}
DEPENDS_dm7020 += "usb2serial"

#all other boxes
DEPENDS = ${ENIGMADEPENDS}
