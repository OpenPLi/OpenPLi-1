export IMAGE_BASENAME = "dreambox-image"

OPENDREAMBOX_COMMON = "base-files busybox \
	ipkg initscripts-opendreambox sysvinit netbase dropbear \
	base-passwd ncurses joe mc vsftpd timezones-alternative \
	netkit-base fakelocale less dreambox-bootlogo  \
	dreambox-keymaps tuxbox-image-info dvbsnoop \
	dreambox-compat tuxbox-common mrouted smartmontools hddtemp"

OPENDREAMBOX_COMMON_D = "util-linux e2fsprogs \
	ppp module-init-tools samba"

OPENDREAMBOX_COMMON_R = "util-linux-sfdisk util-linux-fdisk e2fsprogs-mke2fs \
	e2fsprogs-e2fsck ppp module-init-tools-depmod \
	base-files-doc sambaserver \
	busybox-cron"

#PLi stuff
#enigma:
OPENDREAMBOX_PLI = "pli-files plimgr scramble tuxbox-links"
OPENDREAMBOX_PLI_R = "enigma-plugin-bitrate enigma-plugin-rss enigma-plugin-mv enigma-plugin-plimgr enigma-plugin-movieplayer enigma-locale-fy"

# legacy tuxbox stuff (enigma, plugins, ...)
OPENDREAMBOX_TUXBOX = "enigma ipkgpl"
OPENDREAMBOX_TUXBOX_D = "tuxbox-plugins tuxbox-plugins-enigma links-dream"
OPENDREAMBOX_TUXBOX_R = " \
	tuxbox-plugin-snake     tuxbox-plugin-tuxmail \
	tuxbox-plugin-lcdcirc   tuxbox-plugin-soko      tuxbox-plugin-tuxtxt \
	tuxbox-plugin-sol       tuxbox-plugin-vierg  	tuxbox-plugin-master \
	tuxbox-plugin-solitair  tuxbox-plugin-yahtzee 	tuxbox-plugin-mines  \
	tuxbox-plugin-tank  	tuxbox-plugin-pacman    tuxbox-plugin-tetris \
	tuxbox-plugin-satfind   tuxbox-plugin-tuxcom 	links-dream-plugin \
	links-dream-plugin enigma-plugin-dreamdata"

OPENDREAMBOX_TUXBOX_R_dm600pvr = " \
	tuxbox-plugin-snake     tuxbox-plugin-tuxmail \
	tuxbox-plugin-soko      tuxbox-plugin-tuxtxt \
	tuxbox-plugin-sol       tuxbox-plugin-vierg  	tuxbox-plugin-master \
	tuxbox-plugin-solitair  tuxbox-plugin-yahtzee 	tuxbox-plugin-mines  \
	tuxbox-plugin-tank  	tuxbox-plugin-pacman    tuxbox-plugin-tetris \
	tuxbox-plugin-tuxcom 	links-dream-plugin	enigma-blindscan"

OPENDREAMBOX_TUXBOX_R_dm500plus = " \
	tuxbox-plugin-snake     tuxbox-plugin-tuxmail \
	tuxbox-plugin-soko      tuxbox-plugin-tuxtxt \
	tuxbox-plugin-sol       tuxbox-plugin-vierg  	tuxbox-plugin-master \
	tuxbox-plugin-solitair  tuxbox-plugin-yahtzee 	tuxbox-plugin-mines  \
	tuxbox-plugin-tank  	tuxbox-plugin-pacman    tuxbox-plugin-tetris \
	tuxbox-plugin-tuxcom 	links-dream-plugin	enigma-blindscan"

# dvb api specific stuff
OPENDREAMBOX_V2_ONLY = "dreambox-dvb-tools tuxbox-stream"
OPENDREAMBOX_V3_ONLY = "dreambox-dvb-tools-v3 sctzap dvbtraffic"

# enigma languages
# disabled: enigma-locale-sr enigma-locale-ur
ENIGMA_LANGUAGE = "enigma-locale-cs enigma-locale-da \
	enigma-locale-de enigma-locale-el enigma-locale-es enigma-locale-et \
	enigma-locale-fi enigma-locale-fr enigma-locale-hr enigma-locale-hu \
	enigma-locale-is enigma-locale-it enigma-locale-lt enigma-locale-nl \
	enigma-locale-no enigma-locale-pl enigma-locale-pt enigma-locale-ro \
	enigma-locale-ru enigma-locale-sk enigma-locale-sl \
	enigma-locale-sv enigma-locale-tr enigma-locale-ar"

OPENDREAMBOX_TUXBOX_R += " ${ENIGMA_LANGUAGE}"
OPENDREAMBOX_TUXBOX_R_dm600pvr += " ${ENIGMA_LANGUAGE}"
OPENDREAMBOX_TUXBOX_R_dm500plus += " ${ENIGMA_LANGUAGE}"

FIREWALL_SUPPORT_R = "\
	iptables kernel-module-ip-tables kernel-module-ip-conntrack kernel-module-iptable-filter \
	kernel-module-ipt-conntrack kernel-module-ipt-reject kernel-module-ipt-state"

MODEM_SUPPORT = "enigma-modem"
MODEM_SUPPORT_R = "kernel-module-crc-ccitt kernel-module-ppp-async \
	kernel-module-ppp-generic \
	kernel-module-slhc update-modules"

WLAN_SUPPORT = "wireless-tools wlan-rt73 zd1211b wpa-supplicant"

WLAN_MADWIFI = "madwifi-ng"
WLAN_MADWIFI_R = "madwifi-ng-modules madwifi-ng-tools"

# now machine specific:
OPENDREAMBOX_COMMON_MACHINE_dm600pvr += "${OPENDREAMBOX_V2_ONLY} ${OPENDREAMBOX_TUXBOX} ${MODEM_SUPPORT}"
OPENDREAMBOX_COMMON_MACHINE_R_dm600pvr += "${OPENDREAMBOX_TUXBOX_R} ${MODEM_SUPPORT_R} dreambox-blindscan-utils"
OPENDREAMBOX_COMMON_MACHINE_D_dm600pvr += "${OPENDREAMBOX_TUXBOX_D}"
OPENDREAMBOX_COMMON_MACHINE_dm600pvr += "${OPENDREAMBOX_PLI}"
OPENDREAMBOX_COMMON_MACHINE_R_dm600pvr += "${OPENDREAMBOX_PLI_R} ${FIREWALL_SUPPORT_R}"

OPENDREAMBOX_COMMON_MACHINE_dm500plus += "${OPENDREAMBOX_V2_ONLY} ${OPENDREAMBOX_TUXBOX} ${MODEM_SUPPORT}"
OPENDREAMBOX_COMMON_MACHINE_R_dm500plus += "${OPENDREAMBOX_TUXBOX_R} ${MODEM_SUPPORT_R} dreambox-blindscan-utils"
OPENDREAMBOX_COMMON_MACHINE_D_dm500plus += "${OPENDREAMBOX_TUXBOX_D}"
OPENDREAMBOX_COMMON_MACHINE_dm500plus += "${OPENDREAMBOX_PLI}"
OPENDREAMBOX_COMMON_MACHINE_R_dm500plus += "${OPENDREAMBOX_PLI_R} ${FIREWALL_SUPPORT_R}"

OPENDREAMBOX_COMMON_MACHINE_dm7020 += "${OPENDREAMBOX_V2_ONLY} ${OPENDREAMBOX_TUXBOX} ${MODEM_SUPPORT}"
OPENDREAMBOX_COMMON_MACHINE_R_dm7020 += "${OPENDREAMBOX_TUXBOX_R} ${MODEM_SUPPORT_R}"
OPENDREAMBOX_COMMON_MACHINE_D_dm7020 += "${OPENDREAMBOX_TUXBOX_D}"
OPENDREAMBOX_COMMON_MACHINE_dm7020 += "${OPENDREAMBOX_PLI}"
OPENDREAMBOX_COMMON_MACHINE_R_dm7020 += "${OPENDREAMBOX_PLI_R} ${FIREWALL_SUPPORT_R}"

# collect the stuff into OPENDREAMBOX_COMMON
OPENDREAMBOX_COMMON += " ${OPENDREAMBOX_COMMON_MACHINE}"
OPENDREAMBOX_COMMON_R += " ${OPENDREAMBOX_COMMON_MACHINE_R}"
OPENDREAMBOX_COMMON_D += " ${OPENDREAMBOX_COMMON_MACHINE_D}"

# add bootstrap stuff
DEPENDS = "${OPENDREAMBOX_COMMON} ${BOOTSTRAP_EXTRA_DEPENDS} ${OPENDREAMBOX_COMMON_D}"
RDEPENDS = "${OPENDREAMBOX_COMMON} ${BOOTSTRAP_EXTRA_RDEPENDS} ${OPENDREAMBOX_COMMON_R}"

# we don't want any locales, at least not in the common way.
IMAGE_LINGUAS = " "

export IPKG_INSTALL = '${RDEPENDS}'

inherit image_ipk

export NFO = '${DEPLOY_DIR_IMAGE}/${IMAGE_NAME}.nfo'

do_rootfs_append() {
	printf "Image: ${DISTRO_VERSION}\n" > ${NFO}
	printf "Machine: Dreambox ${MACHINE}\n" >> ${NFO}
	DATE=`date +%Y-%m-%d' '%H':'%M`
	printf "Date: ${DATE}\n" >> ${NFO}
	printf "Issuer: PLi team\n" >> ${NFO}
	if [ "${DESC}" != "" ]; then
		printf "Description: ${DESC}\n" >> ${NFO}
		printf "${DESC}\n" >> ${DEPLOY_DIR_IMAGE}/${IMAGE_NAME}.desc
	fi
	MD5SUM=`md5sum ${DEPLOY_DIR_IMAGE}/${IMAGE_NAME}.nfi | cut -b 1-32`
	printf "MD5: ${MD5SUM}\n" >> ${NFO}
}
