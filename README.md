# ai-utils submodule

This repository is a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
providing C++ utilities for larger projects, including:

* <tt>AIAlert</tt> : an exception based error reporting system.
* <tt>Global</tt> / <tt>Singleton</tt> : template classes for global objects.
* <tt>MultiLoop</tt> : A variable number of nested for loops.
* <tt>Vector</tt> : A wrapper around <tt>std::vector</tt> that only allows specific type as index.
* Several utilities like <tt>at_scope_end</tt>, <tt>double_to_str_precision</tt>, <tt>for_each_until</tt> and convenience macros.

The root project should be using
[autotools](https://en.wikipedia.org/wiki/GNU_Build_System_autotools),
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
where you want to have the <tt>utils</tt> subdirectory:

<pre>
git submodule add https://github.com/CarloWood/ai-utils.git utils
</pre>

This should clone ai-utils into the subdirectory <tt>utils</tt>, or
if you already cloned it there, it should add it.

Changes to <tt>configure.ac</tt> and <tt>Makefile.am</tt>
are taken care of my <tt>cwm4</tt>, except for linking
which works as usual;

for example, a module that defines a

<pre>
bin_PROGRAMS = singlethreaded_foobar multithreaded_foobar
</pre>

would also define

<pre>
singlethreaded_foobar_CXXFLAGS = @LIBCWD_FLAGS@
singlethreaded_foobar_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la

multithreaded_foobar_CXXFLAGS = @LIBCWD_R_FLAGS@
multithreaded_foobar_LDADD = ../utils/libutils_r.la $(top_builddir)/cwds/libcwds_r.la
</pre>

or whatever the path to `utils` is, to link with the required submodules,
libraries, and assuming you also use the [cwds](https://github.com/CarloWood/cwds) submodule.

Finally, run

<pre>
./autogen.sh
</pre>

to let cwm4 do its magic, and commit all the changes.

Checkout [ai-utils-testsuite](https://github.com/CarloWood/ai-utils-testsuite)
for an example of a project that uses this submodule.
