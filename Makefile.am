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
	AIAlert.cxx \
	DelayLoopCalibration.cxx \
	FuzzyBool.cxx \
	GlobalObjectManager.cxx \
	MemoryPagePool.cxx \
	NodeMemoryPool.cxx \
	Signals.cxx \
	debug_ostream_operators.cxx \
	double_to_str_precision.cxx \
	itoa.cxx \
	malloc_size.cxx \
	print_using.cxx \
	translate.cxx \
	threading/aithreadid.cxx \
	threading/Semaphore.cxx \
	threading/SpinSemaphore.cxx

HEADERS = \
	AIAlert.h \
	AIRefCount.h \
	DelayLoopCalibration.h \
	FunctionView.h \
	FuzzyBool.h \
	GlobalObjectManager.h \
	Global.h \
	MemoryPagePool.h \
	NodeMemoryPool.h \
	NodeMemoryResource.h \
	MultiLoop.h \
	SimpleSegregatedStorage.h \
	Signals.h \
	Singleton.h \
	apply_function.h \
	at_scope_end.h \
	cpu_relax.h \
	debug_ostream_operators.h \
	double_to_str_precision.h \
	for_each_until.h \
	is_pointer_like.h \
	is_power_of_two.h \
	itoa.h \
	log2.h \
	macros.h \
	malloc_size.h \
	print_using.h \
	translate.h \
	ulong_to_base.h \
\
	threading/FIFOBuffer.h \
	threading/Futex.h \
	threading/Gate.h \
	threading/MpscQueue.h \
	threading/Semaphore.h \
	threading/SpinSemaphore.h \
	threading/StartingGate.h \
	threading/aithreadid.h

libutils_la_SOURCES = ${SOURCES} ${HEADERS}
libutils_la_CXXFLAGS = @LIBCWD_FLAGS@
libutils_la_LIBADD = @LIBCWD_LIBS@

libutils_r_la_SOURCES = ${SOURCES} ${HEADERS}
libutils_r_la_CXXFLAGS = @LIBCWD_R_FLAGS@
libutils_r_la_LIBADD = @LIBCWD_R_LIBS@

#singleton_test_SOURCES = Singleton_tst.cxx

#singleton_test_CXXFLAGS = @LIBCWD_FLAGS@
#singleton_test_LDADD = libutils.la $(top_builddir)/cwds/libcwds.la

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	-rm -f *.s *.ii config.h.in

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
