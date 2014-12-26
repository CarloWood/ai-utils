EXTRA_DIST = *.h

noinst_LTLIBRARIES = libutils.la
# See src/Makefile.am
DEFS = @DEFS@
DEFAULT_INCLUDES=-iquote $(top_builddir) -iquote $(top_builddir)/src -iquote $(srcdir) -iquote $(srcdir)/.. -iquote $(srcdir)/../cwd

# See src/Makefile.am.
libutils_la_CXXFLAGS = @CXXFLAGS@ @CWD_FLAGS@
libutils_la_SOURCES = \
	debug_ostream_operators.cc \
	AIAlert.cpp \
	translate.cpp \
	double_to_str_precision.cpp \
	GlobalObjectManager.cpp

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

if MAINTAINER_MODE
distclean-local:
	rm -f *.s *.ii

clean-local:
endif
