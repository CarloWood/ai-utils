AM_CPPFLAGS = -iquote $(top_builddir) -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds

noinst_LTLIBRARIES =
if CW_NON_THREADED
noinst_LTLIBRARIES += libutils.la
endif
if CW_THREADED
noinst_LTLIBRARIES += libutils_r.la
endif

SOURCES = \
	debug_ostream_operators.cxx \
	debug_ostream_operators.h \
	AIAlert.cxx \
	AIAlert.h \
	translate.cxx \
	translate.h \
	double_to_str_precision.cxx \
	double_to_str_precision.h \
	GlobalObjectManager.cxx \
	GlobalObjectManager.h \
	Global.h \
	Singleton.h \
	for_each_until.h \
	at_scope_end.h \
	macros.h

libutils_la_SOURCES = ${SOURCES}
libutils_la_CXXFLAGS = @LIBCWD_FLAGS@
libutils_la_LIBADD = @LIBCWD_LIBS@

libutils_r_la_SOURCES = ${SOURCES}
libutils_r_la_CXXFLAGS = @LIBCWD_R_FLAGS@
libutils_r_la_LIBADD = @LIBCWD_R_LIBS@

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	-rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
