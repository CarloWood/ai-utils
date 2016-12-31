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

Add the following line to <tt>configure.ac</tt>:

<pre>
CW_SUBMODULE([<i>src</i>], [utils])
</pre>

where <code><i>src</i>/utils</code> is the path relative to the projects root (where `configure.ac` is).
If the first argument is empty, it can be omitted (<code>CW_SUBMODULE([utils])</code>).

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
