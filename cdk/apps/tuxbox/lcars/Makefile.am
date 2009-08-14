AUTOMAKE_OPTIONS = gnu

SUBDIRS = src

install-data-local:
	install -d $(FONTDIR)
	install -m 0644 data/fonts/*.ttf $(FONTDIR)
	install -d $(CONFIGDIR)/lcars
	install -d $(DATADIR)/lcars
	install -d $(DATADIR)/lcars/http
	install -m 0644 data/scan-file/scanlist.dat $(CONFIGDIR)/lcars
	install -m 0644 data/skin/*.lcars $(CONFIGDIR)/lcars
	install -m 0644 data/skin/*.raw $(DATADIR)/lcars
	install -m 0644 data/http/*.gif $(DATADIR)/lcars/http
	install -m 0644 data/http/*.htm $(DATADIR)/lcars/http
	install -m 0644 data/http/*.css $(DATADIR)/lcars/http
	install -m 0644 data/fonts/*.ttf $(DATADIR)/lcars/http
	install -m 0644 data/extras/*.conf $(CONFIGDIR)/lcars
