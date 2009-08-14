lib_LTLIBRARIES = libcrypto++.la

libcrypto___la_SOURCES = \
	bio.cpp \
	evp.cpp \
	lib.cpp \
	rand.cpp \
	rsa.cpp \
	x509.cpp

libcrypto___la_LDFLAGS = -lcrypto

AM_CXXFLAGS = -fno-rtti

INCLUDES = -I$(top_builddir)/../apps -I$(top_builddir)/../apps/libcrypto++

all-local:
	@ln -s $(top_builddir)/../apps/libcrypto++/*.hpp .

mostlyclean-local:
	@rm -r *.cpp *.hpp

$(libcrypto___la_SOURCES):
	@ln -s $(top_builddir)/../apps/libcrypto++/$@
