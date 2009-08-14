INCLUDES = -I$(DRIVER)/include

bin_PROGRAMS = grab

grab_SOURCES = main.c 

FLAGS = -DREGPARAM="__attribute__((regparm(3)))" -DKBD_LANG=0
CFLAGS = @CFLAGS@ $(FLAGS)
CXXFLAGS = @CXXFLAGS@ $(FLAGS)
LDFLAGS = -lm -lz -lpng @LDFLAGS@

