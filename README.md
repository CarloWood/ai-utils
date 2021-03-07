# ai-utils submodule

This repository is a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
providing C++ utilities for larger projects, including:

* <tt>AIAlert</tt> : an exception based error reporting system.
* <tt>AIFIFOBuffer</tt> : A spsc lock-free ring buffer for trivially copyable objects.
* <tt>AIRefCount</tt> : Base class for classes that need to wrapped into as boost::intrusive_ptr.
* <tt>AISignals</tt> : C++ wrapper around POSIX signals.
* <tt>Array</tt> / <tt>Vector</tt> : A wrapper around <tt>std::array</tt> / <tt>std::vector</tt> that only allow a specific type as index.
* <tt>AtomicFuzzyBool</tt> / <tt>FuzzyBool</tt> : Fuzzy booleans; great for conditions that are subject to races in a multi-threaded application.
* <tt>BitSet<T></tt> : A wrapper around unsigned integral types T that allows fast bit-level manipulation, including iterating in a loop over all set bits.
* <tt>DelayLoopCalibration</tt> : Determine the required loop size for a given lambda to delay the code a given amount of milliseconds.
* <tt>Dictionary</tt> : Map known words to known enum values, and unknown words to new (different) values.
* <tt>FunctionView</tt> : Cheap, lightweight Callable (like std::function) suitable for passing arbitrary functions as argument.
* <tt>Global</tt> / <tt>Singleton</tt> : template classes for global objects.
* <tt>InstanceTracker</tt> : Base class to keep track of all existing objects of a given type.
* <tt>iomanip</tt> : Custom io manipulators.
* <tt>itoa</tt> : Maximum speed integer to string converter.
* <tt>MemoryPagePool</tt> : A memory pool that returns fixed-size memory blocks allocated with std::aligned_alloc and aligned to memory_page_size.
* <tt>MultiLoop</tt> : A variable number of nested for loops.
* <tt>NodeMemoryResource</tt> : A fixed size memory resource that uses a MemoryPagePool as upstream.
* <tt>RandomStream</tt> : Stream producing random characters.
* <tt>Register</tt> : Register callbacks for global objects, to be called once main() is entered.
* <tt>REMOVE_TRAILING_COMMA</tt> : Macro that removes the last (possibly empty) argument.
* <tt>SimpleSegregatedStorage</tt> : Maintains an unordered free list of blocks (used by NodeMemoryResource and MemoryPagePool).
* <tt>StreamHasher</tt> : Calculate a digest of input written using operator<<.
* <tt>VTPtr</tt> : Custom virtual table for classes. The advantage being that the virtual table is dynamic and can be altered during runtime.

* Several utilities like <tt>almost_equal</tt>, <tt>at_scope_end</tt>, <tt>c_escape</tt>, <tt>clz / ctz / mssb / popcount</tt>,
  <tt>constexpr_ceil</tt>, <tt>cpu_relax</tt>, <tt>double_to_str_precision</tt>, <tt>for_each_until</tt>,
  <tt>is_pointer_like</tt>, <tt>is_power_of_two</tt>, <tt>log2</tt>, <tt>malloc_size</tt>,
  <tt>nearest_multiple_of_power_of_two</tt>, <tt>nearest_power_of_two</tt>, <tt>print_using</tt> <tt>reversed</tt>,
  <tt>split</tt>, <tt>ulong_to_base</tt> and convenience macros.

The root project should be using
[cmake](https://cmake.org/overview/)
[cwm4](https://github.com/CarloWood/cwm4) and
[libcwd](https://github.com/CarloWood/libcwd).

## Checking out a project that uses the ai-utils submodule.

To clone a project example-project that uses ai-utils simply run:

<pre>
<b>git clone --recursive</b> &lt;<i>URL-to-project</i>&gt;<b>/example-project.git</b>
<b>cd example-project</b>
<b>AUTOGEN_CMAKE_ONLY=1 ./autogen.sh</b>
</pre>

The <tt>--recursive</tt> is optional because <tt>./autogen.sh</tt> will fix
it when you forgot it.

When using [GNU autotools](https://en.wikipedia.org/wiki/GNU_Autotools) you should of course
not set <tt>AUTOGEN_CMAKE_ONLY</tt>. Also, you probably want to use <tt>--enable-mainainer-mode</tt>
as option to the generated <tt>configure</tt> script.

In order to use <tt>cmake</tt> configure as usual, for example to build with 8 cores a debug build:

<pre>
<b>mkdir build_debug</b>
<b>cmake -S . -B build_debug -DCMAKE_MESSAGE_LOG_LEVEL=DEBUG -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON -DEnableDebugGlobal:BOOL=OFF</b>
<b>cmake --build build_debug --config Debug --parallel 8</b>
</pre>

Or to make a release build:

<pre>
<b>mkdir build_release</b>
<b>cmake -S . -B build_release -DCMAKE_BUILD_TYPE=Release</b>
<b>cmake --build build_release --config Release --parallel 8</b>
</pre>

## Adding the ai-utils submodule to a project

To add this submodule to a project, that project should already
be set up to use [cwm4](https://github.com/CarloWood/cwm4).

Simply execute the following in a directory of that project
where you want to have the <tt>utils</tt> subdirectory (the
root of the project is recommended as that is the only thing
I've tested so far):

<pre>
git submodule add https://github.com/CarloWood/ai-utils.git utils
</pre>

This should clone ai-utils into the subdirectory <tt>utils</tt>, or
if you already cloned it there, it should add it.

### Using cmake

Check out the submodules [cwds](https://github.com/CarloWood/cwds) and [cwm4](https://github.com/CarloWood/cwm4) in the root of the project:

<pre>
git submodule add https://github.com/CarloWood/cwds.git
git submodule add https://github.com/CarloWood/cwm4.git
</pre>

The easiest way to use libcwd is by using [gitache](https://github.com/CarloWood/gitache).

For that to happen create in the root of the project (that uses utils)
a directory <tt>cmake/gitache-configs</tt> and put in it the file <tt>libcwd_r.cmake</tt>
with the content:

<pre>
gitache_config(
  GIT_REPOSITORY
    "https://github.com/CarloWood/libcwd.git"
  GIT_TAG
    "master"
  CMAKE_ARGS
    "-DEnableLibcwdAlloc:BOOL=OFF -DEnableLibcwdLocation:BOOL=ON"
)
</pre>

Add the variable <tt>GITACHE_ROOT</tt> to your environment,
for example add to your <tt>~/.bashrc</tt> the line:

<pre>
export GITACHE_ROOT="/opt/gitache"
</pre>

Add the following lines to the <tt>CMakeLists.txt</tt> in the
root of the project (directly under the <tt>project</tt> line):

<pre>
# Begin of gitache configuration.
set(GITACHE_PACKAGES libcwd_r)
include(cwm4/cmake/StableGitache)
# End of gitache configuration.

include(cwm4/cmake/AICxxProject)

# Exit if someone tries to contaminate the source directory with an in-source build.
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Please do out-of-source builds."
                      "Cleanup: \"rm -rf CMake CMakeCache.txt CMakeFiles/\"")
endif ()

include(AICxxSubmodules)
</pre>

<tt>add_subdirectory</tt> is not necessary for <tt>cwds</tt>, <tt>cwm4</tt> or <tt>utils</tt>.

See for example the root [MakeLists.txt](https://github.com/CarloWood/ai-utils-testsuite/blob/master/CMakeLists.txt) of of ai-utils-testsuite.

Finally, linking is done by adding <tt>${AICXX_OBJECTS_LIST}</tt> to
the appropriate <tt>target_link_libraries</tt>.

See this [MakeLists.txt](https://github.com/CarloWood/ai-utils-testsuite/blob/master/src/CMakeLists.txt)
of ai-utils-testsuite as example.

### Using GNU autotools

Changes to <tt>configure.ac</tt> and <tt>Makefile.am</tt>
are taken care of by <tt>cwm4</tt>, except for linking
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
