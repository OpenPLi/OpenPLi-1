INCLUDES = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/src \
	@CONFIGFILE_CFLAGS@ \
	@EVENTSERVER_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@NET_CFLAGS@ \
	@ZAPIT_CFLAGS@

noinst_LIBRARIES = libneutrino_gui_widget_basic.a libneutrino_gui_widget.a libneutrino_gui_widget2.a 

libneutrino_gui_widget_a_SOURCES = \
	colorchooser.cpp keychooser.cpp \
	menue.cpp stringinput.cpp stringinput_ext.cpp \
	messagebox.cpp hintbox.cpp listbox.cpp \
	lcdcontroler.cpp
	
libneutrino_gui_widget2_a_SOURCES = \
	progresswindow.cpp

libneutrino_gui_widget_basic_a_SOURCES = \
        component.cpp window.cpp

