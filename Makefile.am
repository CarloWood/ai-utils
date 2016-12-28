EXTRA_DIST = *.h

AM_CPPFLAGS = -iquote $(top_srcdir)

noinst_LTLIBRARIES = libutils.la
libutils_la_CXXFLAGS = @CXXFLAGS@ @LIBCWD_FLAGS@
libutils_la_SOURCES = \
	debug_ostream_operators.cpp \
	AIAlert.cpp \
	translate.cpp \
	double_to_str_precision.cpp \
	GlobalObjectManager.cpp

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
