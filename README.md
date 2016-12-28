# ai-utils submodule

This repository is intended to be used as a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
in a larger project that is using
[autotools](https://en.wikipedia.org/wiki/GNU_Build_System autotools),
[cwm4](https://github.com/CarloWood/cwm4) and
[libcwd](https://github.com/CarloWood/libcwd).

## Adding ai-utils to a project

### configure.ac

Add the `Makefile` of ai-utils, to the
[`AC_CONFIG_FILES`](https://www.gnu.org/software/automake/manual/html_node/Requirements.html)
line of `configure.ac` in the root of the project.

For example,

<pre>
AC_CONFIG_FILES([...]
                 ...
                 [<i>src</i>/utils/Makefile]
                 ...)
</pre>

where <code><i>src</i>/utils</code> is the path relative to the projects root (where `configure.ac` is).

Furthermore, `configure.ac` needs to contain something like:

<pre>
dnl Because we use cwautomacros.
CW_AUTOMACROS

dnl Add --enable-debug (DEBUG, DOXYGEN_DEBUG), --enable-libcwd (CWDEBUG, DOXYGEN_CWDEBUG),
dnl --enable-optimise and --enable-profile options. Update USE_LIBCWD, CWD_LIBS and CXXFLAGS accordingly.
CW_OPG_FLAGS([-W -Wall -Woverloaded-virtual -Wundef -Wpointer-arith -Wwrite-strings -Winline])

dnl This source code is C++11 and thread-safe.
CXXFLAGS="$CXXFLAGS -pthread -std=c++11"
LIBCWD_FLAGS="$CWD_R_FLAGS"
LIBCWD_LIBS="$CWD_R_LIBS"
AC_SUBST(LIBCWD_FLAGS)
AC_SUBST(LIBCWD_LIBS)
</pre>

This is because ai-utils's `Makefile.am` contains the substitution templates
`@LIBCWD_FLAGS@`, `@LIBCWD_LIBS@` and `@DEFS@`.

These are all the requirements needed for the dependency
on `cwautomacros` (which has to be [installed seperately](https://github.com/CarloWood/cwautomacros))
and `libcwd`, except that the latter also means you have to supply a `debug.h`
and `sys.h` (see the [libcwd documentation](http://libcwd.sourceforge.net/www/quickreference.html)).

### Makefile.am

Also add <code>utils</code> to `SUBDIRS` of the `Makefile.am`
of the parent directory (ie <code><i>src</i>/Makefile.am</code>).
For example,

<pre>
SUBDIRS = utils
</pre>

A directory with source files that include headers
from the ai-utils submodule (as in <code>#include "utils/AIAlert.h"</code>, for example)
must include that <code><i>src</i></code> path.

For example, the same <code><i>src</i>/Makefile.am</code> could contain:

<pre>
DEFAULT_INCLUDES=-iquote $(top_builddir) -iquote $(top_builddir)/<i>src</i> -iquote $(srcdir)
</pre>

Where that last `-iquote $(srcdir)` which will allow `#include "utils/AIAlert.h"`
in source files in <code><i>src</i></code>, while a <code><i>src/foo</i>/Makefile.am</code>
that wanted to use <code><i>src</i>/utils</code> would do:

<pre>
DEFAULT_INCLUDES=-iquote $(top_builddir) -iquote $(top_builddir)/<i>src</i> -iquote $(srcdir)/..
</pre>

Linking works as usual. For example a module that defines a

<pre>
bin_PROGRAMS = foobar
</pre>

would also define

<pre>
foobar_LDADD = ../utils/.libs/libutils.a
foobar_DEPENDENCIES = ../utils/libutils.la
</pre>

or whatever the path to `utils` is.
