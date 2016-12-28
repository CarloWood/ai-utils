# ai-utils submodule

This repository is a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
providing C++ utilities for larger projects, including:

* <tt>AIAlert</tt> : an exception based error reporting system.
* <tt>Global</tt> / <tt>Singleton</tt> : template classes for global objects.
* Several utilities like <tt>at_scope_end</tt>, <tt>double_to_str_precision</tt>, <tt>for_each_until</tt> and convenience macros.

The root project should be using
[autotools](https://en.wikipedia.org/wiki/GNU_Build_System autotools),
[cwm4](https://github.com/CarloWood/cwm4) and
[libcwd](https://github.com/CarloWood/libcwd).

## Checking out a project that uses the ai-utils submodule.

To clone a project example-project that uses ai-utils simply run:

<pre>
<b>git clone --recursive</b> &lt;<i>URL-to-project</i>&gt;<b>/example-project.git</b>
<b>cd example-project</b>
<b>./autogen.sh</b>
</pre>

The <tt>--recursive</tt> is optional because <tt>./autogen.sh</tt> will fix
it when you forgot it.

Afterwards you probably want to use <tt>--enable-mainainer-mode</tt>
as option to the generated <tt>configure</tt> script.

## Adding the ai-utils submodule to a project

To add this submodule to a project, that project should already
be set up to use [cwm4](https://github.com/CarloWood/cwm4).

Simply execute the following in a directory of that project
where you what to have the <tt>utils</tt> subdirectory:

<pre>
git submodule add https://github.com/CarloWood/ai-utils.git utils
</pre>

This should clone ai-utils into the subdirectory <tt>utils</tt>, or
if you already cloned it there, it should add it.

You also need to make changes to autotool files:

### configure.ac

Add the `Makefile` of ai-utils, to the
[`AC_CONFIG_FILES`](https://www.gnu.org/software/automake/manual/html_node/Requirements.html)
line of `configure.ac` in the root of the project.

For example, add the line in bold:

<pre>
AC_CONFIG_FILES([...]
                 ...
                 <b>[<i>src</i>/utils/Makefile]</b>
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

This is because ai-utils's `Makefile.am` contains the substitution templates `@LIBCWD_FLAGS@` and `@LIBCWD_LIBS@`.
Of course you are free to use different warning options and/or comments.

These are all the requirements needed for the dependency
on `cwm4` and `libcwd`, except that the latter also means you have to supply a `debug.h`
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
AM_CPPFLAGS = -iquote $(srcdir)
</pre>

Where that `-iquote $(srcdir)` which will allow `#include "utils/AIAlert.h"`
in source files in <code><i>src</i></code>, while a <code><i>src/foo</i>/Makefile.am</code>
that wanted to use <code><i>src</i>/utils</code> would do:

<pre>
AM_CPPFLAGS = -iquote $(srcdir)/..
</pre>

Linking works as usual. For example a module that defines a

<pre>
bin_PROGRAMS = foobar
</pre>

would also define

<pre>
foobar_LDADD = ../utils/libutils.la
</pre>

or whatever the path to `utils` is.

Finally, run

<pre>
./autogen.sh
</pre>

and commit all your changes.
