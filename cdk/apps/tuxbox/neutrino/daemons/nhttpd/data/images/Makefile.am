EXTRA_DIST = \
	arrowdown.gif arrowup.gif bg.gif bottom.gif elist.gif \
	graph_bar.gif graph_bg.gif graph_left.gif graph_right.gif \
	green_dot.gif hidden.gif left.gif lock.gif modify.gif \
	new.gif nokia.gif record.gif reload.gif remove.gif \
	right.gif sagem.gif timer2.gif timer.gif top.gif \
	unlock.gif visible.gif \ 
	left_01.jpg left_02.jpg left_03.jpg left_04.jpg left_05.jpg \
	left_06.jpg left_07.jpg left_08.jpg left_09.jpg left_10.jpg \
	muted_off.jpg muted_on.jpg mute_off.jpg mute_on.jpg \
	radio_off.jpg radio_on.jpg shutdown.jpg shutdown_off.jpg \
	shutdown_on.jpg tv_off.jpg tv_on.jpg vol_flashed.jpg \
	volmin_off.jpg volmin_on.jpg volplu_off.jpg volplu_on.jpg \
	vol_unflashed.jpg

install-data-local:
	install -d $(DATADIR)/neutrino/httpd/images
	install -m 0644 *.gif $(DATADIR)/neutrino/httpd/images
	install -m 0644 *.jpg $(DATADIR)/neutrino/httpd/images

uninstall-local:
	-rm -rf $(DATADIR)/neutrino/httpd/images
