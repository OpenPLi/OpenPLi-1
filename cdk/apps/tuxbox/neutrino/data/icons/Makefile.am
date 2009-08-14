EXTRA_DIST = \
	audio.raw blau.raw border_lr.raw border_ul.raw colors.raw dboxradio.raw \
	games.raw gelb.raw gruen.raw keybinding.raw language.raw mainmenue.raw \
	mute.raw network.raw radar0.raw radar1.raw radar2.raw radar3.raw \
	radar4.raw radar5.raw radar6.raw radar7.raw radar8.raw radar9.raw \
	rot.raw settings.raw shutdown.raw video.raw volume.raw volumebody.raw \
	volumeslider2.raw volumeslider2alpha.raw volumeslider2blue.raw \
	volumeslider2green.raw volumeslider2red.raw

install-data-local:
	install -d $(DATADIR)/neutrino/icons
	install -m 0644 $(srcdir)/*.raw $(DATADIR)/neutrino/icons
	install -m 0644 $(srcdir)/*.pal $(DATADIR)/neutrino/icons

uninstall-local:
	-rm -rf $(DATADIR)/neutrino/icons
	-rmdir $(DATADIR)/neutrino
