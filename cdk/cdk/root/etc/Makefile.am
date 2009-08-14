SUBDIRS = init.d

FILES = \
	fstab host.conf inetd.conf inittab issue.net mime.types \
	nsswitch.conf profile protocols rpc services termcap timezone.xml \
	vsftpd.conf

VAR_ETC_LINKS = \
	exports group hostname hosts localtime passwd resolv.conf smb.conf

EXTRA_DIST = $(FILES) $(VAR_ETC_LINKS) mtab

all-local: $(FILES)

install-data-local: $(FILES)
	install -d $(targetprefix)/etc
	@for i in $(FILES) ; do \
		install -m644 $$i $(targetprefix)/etc ; \
	done
	@for i in $(VAR_ETC_LINKS) ; do \
		rm -f $(targetprefix)/$$i; \
		ln -sf /var/etc/$$i $(targetprefix)/etc ; \
	done
	rm -f $(targetprefix)/etc/mtab
	ln -sf /proc/mounts $(targetprefix)/etc/mtab
