SUBDIRS = images

EXTRA_DIST = \
	controlpanel.html dbox.html epg.html index.html \
	settings.html shutdown.html \
	global.css \
	controlpanel.include1 controlpanel.include2 controlpanel.include \
	channels.js

install-data-local:
	install -d $(DATADIR)/neutrino/httpd
	install -m 0644 *.html $(DATADIR)/neutrino/httpd
	install -m 0644 *.css $(DATADIR)/neutrino/httpd
	install -m 0644 *.incl* $(DATADIR)/neutrino/httpd
	install -m 0644 *.js $(DATADIR)/neutrino/httpd

uninstall-local:
	-rm -rf $(DATADIR)/neutrino/httpd
