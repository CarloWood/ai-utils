AM_CPPFLAGS = -iquote $(top_builddir) -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds

noinst_LTLIBRARIES =
if CW_NON_THREADED
noinst_LTLIBRARIES += libutils.la
endif
if CW_THREADED
noinst_LTLIBRARIES += libutils_r.la
endif

# In order to compile this, add -DDEBUGGLOBAL to CXXFLAGS and recompile everything.
#bin_PROGRAMS = singleton_test

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
	NodeMemoryPool.cxx \
	NodeMemoryPool.h \
	itoa.cxx \
	itoa.h \
	sbll.cxx \
	sbll.h \
	AIFIFOBuffer.h \
	AIRefCount.h \
	FunctionView.h \
	MultiLoop.h \
	apply_function.h \
	at_scope_end.h \
	for_each_until.h \
	is_power_of_two.h \
	macros.h \
	ulong_to_base.h

libutils_la_SOURCES = ${SOURCES}
libutils_la_CXXFLAGS = -std=c++11 @LIBCWD_FLAGS@
libutils_la_LIBADD = @LIBCWD_LIBS@

libutils_r_la_SOURCES = ${SOURCES}
libutils_r_la_CXXFLAGS = -std=c++11 @LIBCWD_R_FLAGS@
libutils_r_la_LIBADD = @LIBCWD_R_LIBS@

#singleton_test_SOURCES = Singleton_tst.cxx

#singleton_test_CXXFLAGS = @LIBCWD_FLAGS@
#singleton_test_LDADD = libutils.la $(top_builddir)/cwds/libcwds.la

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	-rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
