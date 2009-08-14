DOMAIN = $(PACKAGE)
XGETTEXT_OPTIONS = --keyword=_ --keyword=N_ --from-code=utf-8
COPYRIGHT_HOLDER = tuxbox project
EXTRA_LOCALE_CATEGORIES =

localedir = $(datadir)/locale

-include POTFILES

POTFILES: $(srcdir)/POTFILES.in
	@echo "POTFILES = \\" > POTFILES
	@sed -e "/^#/d" -e "/^[ ]*\$$/d" -e "s,.*,	$(top_srcdir)/& \\\\," -e "\$$s/\(.*\) \\\\/\1/" < $(srcdir)/POTFILES.in >> POTFILES

SUFFIXES = .po .gmo .mo .sed .sin .nop .po-update

.po.mo:
	@echo "$(MSGFMT) -c -o $@ $<"; \
	$(MSGFMT) -c -o t-$@ $< && mv t-$@ $@

.po.gmo:
	@lang=`echo $* | sed -e 's,.*/,,'`; \
	test "$(srcdir)" = . && cdcmd="" || cdcmd="cd $(srcdir) && "; \
	echo "$${cdcmd}rm -f $${lang}.gmo && @GMSGFMT@ -c --statistics -o $${lang}.gmo $${lang}.po"; \
	cd $(srcdir) && rm -f $${lang}.gmo && @GMSGFMT@ -c --statistics -o t-$${lang}.gmo $${lang}.po && (mv t-$${lang}.gmo $${lang}.gmo  || /bin/true)

.nop.po-update:
	@lang=`echo $@ | sed -e 's/\.po-update$$//'`; \
	if test "$(PACKAGE)" = "gettext"; then PATH=`pwd`/../src:$$PATH; fi; \
	tmpdir=`pwd`; \
	echo "$$lang:"; \
	test "$(srcdir)" = . && cdcmd="" || cdcmd="cd $(srcdir) && "; \
	echo "$${cdcmd}@MSGMERGE@ $$lang.po $(DOMAIN).pot -o $$lang.new.po"; \
	cd $(srcdir); \
	if @MSGMERGE@ $$lang.po $(DOMAIN).pot -o $$tmpdir/$$lang.new.po; then \
	  if cmp $$lang.po $$tmpdir/$$lang.new.po >/dev/null 2>&1; then \
	    rm -f $$tmpdir/$$lang.new.po; \
	  else \
	    if mv -f $$tmpdir/$$lang.new.po $$lang.po; then \
	      :; \
	    else \
	      echo "msgmerge for $$lang.po failed: cannot move $$tmpdir/$$lang.new.po to $$lang.po" 1>&2; \
	      exit 1; \
	    fi; \
	  fi; \
	else \
	  echo "msgmerge for $$lang.po failed!" 1>&2; \
	  rm -f $$tmpdir/$$lang.new.po; \
	fi


all: all-@USE_NLS@

all-yes: $(CATALOGS)
all-no:

# Note: Target 'all' must not depend on target '$(DOMAIN).pot-update',
# otherwise packages like GCC can not be built if only parts of the source
# have been downloaded.

$(DOMAIN).pot-update: $(POTFILES) $(srcdir)/POTFILES.in
	$(srcdir)/xml2po $(srcdir)/POTFILES.xml $(top_srcdir) > $(DOMAIN).po
	@XGETTEXT@ --default-domain=$(DOMAIN) --directory=$(top_srcdir) \
	  --add-comments=TRANSLATORS: $(XGETTEXT_OPTIONS) \
	  --files-from=$(srcdir)/POTFILES.in \
	  --copyright-holder='$(COPYRIGHT_HOLDER)' \
	  --join-existing
	test ! -f $(DOMAIN).po || { \
	  if test -f $(srcdir)/$(DOMAIN).pot; then \
	    sed -e 's/^"POT-Creation-Date: .*\"$$//' < $(srcdir)/$(DOMAIN).pot > $(DOMAIN).1po && \
	    sed -e 's/^"POT-Creation-Date: .*"$$//' < $(DOMAIN).po > $(DOMAIN).2po && \
	    if cmp $(DOMAIN).1po $(DOMAIN).2po >/dev/null 2>&1; then \
	      rm -f $(DOMAIN).1po $(DOMAIN).2po $(DOMAIN).po; \
	    else \
	      rm -f $(DOMAIN).1po $(DOMAIN).2po $(srcdir)/$(DOMAIN).pot && \
	      mv $(DOMAIN).po $(srcdir)/$(DOMAIN).pot; \
	    fi; \
	  else \
	    mv $(DOMAIN).po $(srcdir)/$(DOMAIN).pot; \
	  fi; \
	}

$(srcdir)/$(DOMAIN).pot:
	$(MAKE) $(DOMAIN).pot-update

@POFILES@: $(srcdir)/$(DOMAIN).pot
	@lang=`echo $@ | sed -e 's,.*/,,' -e 's/\.po$$//'`; \
	test "$(srcdir)" = . && cdcmd="" || cdcmd="cd $(srcdir) && "; \
	echo "$${cdcmd}@MSGMERGE@ --update $${lang}.po $(DOMAIN).pot"; \
	cd $(srcdir) && @MSGMERGE@ --update $${lang}.po $(DOMAIN).pot

@DUMMYPOFILES@:

install-data-local: install-data-@USE_NLS@

install-data-no: all
install-data-yes: all
	$(mkinstalldirs) $(DESTDIR)$(localedir)
	cp -pv $(srcdir)/locale.alias $(DESTDIR)$(localedir)
	@catalogs='@CATALOGS@'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed -e 's/\.gmo$$//'`; \
	  dir=$(localedir)/$$lang/LC_MESSAGES; \
	  $(mkinstalldirs) $(DESTDIR)$$dir; \
	  if test -r $$cat; then realcat=$$cat; else realcat=$(srcdir)/$$cat; fi; \
	  $(INSTALL_DATA) $$realcat $(DESTDIR)$$dir/$(DOMAIN).mo; \
	  echo "installing $$realcat as $(DESTDIR)$$dir/$(DOMAIN).mo"; \
	  for lc in '' $(EXTRA_LOCALE_CATEGORIES); do \
	    if test -n "$$lc"; then \
	      if (cd $(DESTDIR)$(localedir)/$$lang && LC_ALL=C ls -l -d $$lc 2>/dev/null) | grep ' -> ' >/dev/null; then \
	        link=`cd $(DESTDIR)$(localedir)/$$lang && LC_ALL=C ls -l -d $$lc | sed -e 's/^.* -> //'`; \
	        mv $(DESTDIR)$(localedir)/$$lang/$$lc $(DESTDIR)$(localedir)/$$lang/$$lc.old; \
	        mkdir $(DESTDIR)$(localedir)/$$lang/$$lc; \
	        (cd $(DESTDIR)$(localedir)/$$lang/$$lc.old && \
	         for file in *; do \
	           if test -f $$file; then \
	             ln -s ../$$link/$$file $(DESTDIR)$(localedir)/$$lang/$$lc/$$file; \
	           fi; \
	         done); \
	        rm -f $(DESTDIR)$(localedir)/$$lang/$$lc.old; \
	      else \
	        if test -d $(DESTDIR)$(localedir)/$$lang/$$lc; then \
	          :; \
	        else \
	          rm -f $(DESTDIR)$(localedir)/$$lang/$$lc; \
	          mkdir $(DESTDIR)$(localedir)/$$lang/$$lc; \
	        fi; \
	      fi; \
	      rm -f $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo; \
	      ln -s ../LC_MESSAGES/$(DOMAIN).mo $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo 2>/dev/null || \
	      ln $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(DOMAIN).mo $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo 2>/dev/null || \
	      cp -p $(DESTDIR)$(localedir)/$$lang/LC_MESSAGES/$(DOMAIN).mo $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo; \
	      echo "installing $$realcat link as $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo"; \
	    fi; \
	  done; \
	done

uninstall-local: uninstall-@USE_NLS@
uninstall-local-no:
uninstall-local-yes:
	catalogs='@CATALOGS@'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed -e 's/\.gmo$$//'`; \
	  for lc in LC_MESSAGES $(EXTRA_LOCALE_CATEGORIES); do \
	    rm -f $(DESTDIR)$(localedir)/$$lang/$$lc/$(DOMAIN).mo; \
	  done; \
	done

mostlyclean-local:
	rm -f remove-potcdate.sed
	rm -f lib lib.* $(DOMAIN).po $(DOMAIN).1po $(DOMAIN).2po *.new.po
	rm -fr *.o

distclean-local:
	rm -f POTFILES *.mo

maintainer-clean-local:
	rm -f @GMOFILES@

dist-local:
	$(MAKE) update-po
	@$(MAKE) dist2

dist2: $(DISTFILES)
	dists="$(DISTFILES)"; \
	if test -f $(srcdir)/ChangeLog; then dists="$$dists ChangeLog"; fi; \
	if test -f $(srcdir)/LINGUAS; then dists="$$dists LINGUAS"; fi; \
	for file in $$dists; do \
	  if test -f $$file; then \
	    cp -p $$file $(distdir); \
	  else \
	    cp -p $(srcdir)/$$file $(distdir); \
	  fi; \
	done

update-po: Makefile
	$(MAKE) $(DOMAIN).pot-update
	$(MAKE) @UPDATEPOFILES@
	$(MAKE) update-gmo

update-gmo: Makefile @GMOFILES@
	@:

