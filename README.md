# ai-utils submodule

This repository is a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
providing C++ utilities for larger projects, including:

* ``AIAlert`` : an exception based error reporting system.
* ``AIFIFOBuffer`` : A spsc lock-free ring buffer for trivially copyable objects.
* ``AIRefCount`` : Base class for classes that need to wrapped into as ``boost::intrusive_ptr``.
* ``AISignals`` : C++ wrapper around POSIX signals.
* ``Array`` / ``Vector`` : A wrapper around ``std::array`` / ``std::vector`` that only allow a specific type as index.
* ``AtomicFuzzyBool`` / ``FuzzyBool`` : Fuzzy booleans; great for conditions that are subject to races in a multi-threaded application.
* ``BitSet<T>`` : A wrapper around unsigned integral types T that allows fast bit-level manipulation, including iterating in a loop over all set bits.
* ``DelayLoopCalibration`` : Determine the required loop size for a given lambda to delay the code a given amount of milliseconds.
* ``Dictionary`` : Map known words to known enum values, and unknown words to new (different) values.
* ``FunctionView`` : Cheap, lightweight Callable (like std::function) suitable for passing arbitrary functions as argument.
* ``Global`` / ``Singleton`` : template classes for global objects.
* ``InstanceTracker`` : Base class to keep track of all existing objects of a given type.
* ``iomanip`` : Custom io manipulators.
* ``itoa`` : Maximum speed integer to string converter.
* ``MemoryPagePool`` : A memory pool that returns fixed-size memory blocks allocated with ``std::aligned\_alloc`` and aligned to ``memory\_page\_size``.
* ``MultiLoop`` : A variable number of nested for loops.
* ``NodeMemoryResource`` : A fixed size memory resource that uses a MemoryPagePool as upstream.
* ``RandomStream`` : Stream producing random characters.
* ``Register`` : Register callbacks for global objects, to be called once main() is entered.
* ``REMOVE\_TRAILING\_COMMA`` : Macro that removes the last (possibly empty) argument.
* ``SimpleSegregatedStorage`` : Maintains an unordered free list of blocks (used by NodeMemoryResource and MemoryPagePool).
* ``StreamHasher`` : Calculate a digest of input written using operator<<.
* ``VTPtr`` : Custom virtual table for classes. The advantage being that the virtual table is dynamic and can be altered during runtime.

* Several utilities like ``almost\_equal``, ``at\_scope\_end``, ``c\_escape``, ``clz / ctz / mssb / popcount``,
  ``constexpr\_ceil``, ``cpu\_relax``, ``double\_to\_str\_precision``, ``for\_each\_until``,
  ``is\_pointer\_like``, ``is\_power\_of\_two``, ``log2``, ``malloc\_size``,
  ``nearest\_multiple\_of\_power\_of\_two``, ``nearest\_power\_of\_two``, ``print\_using`` ``reversed``,
  ``split``, ``ulong\_to\_base`` and convenience macros.

The root project should be using
[cmake](https://cmake.org/overview/)
[cwm4](https://github.com/CarloWood/cwm4) and
[libcwd](https://github.com/CarloWood/libcwd).

## Checking out a project that uses the ai-utils submodule.

To clone a project example-project that uses ai-utils simply run:

    <b>git clone --recursive</b> &lt;<i>URL-to-project</i>&gt;<b>/example-project.git</b>
    <b>cd example-project</b>
    <b>AUTOGEN_CMAKE_ONLY=1 ./autogen.sh</b>

The ``--recursive`` is optional because ``./autogen.sh`` will fix
it when you forgot it.

When using [GNU autotools](https://en.wikipedia.org/wiki/GNU_Autotools) you should of course
not set ``AUTOGEN\_CMAKE\_ONLY``. Also, you probably want to use ``--enable-mainainer-mode``
as option to the generated ``configure`` script.

In order to use ``cmake`` configure as usual, for example to build with 8 cores a debug build:

    <b>mkdir build_debug</b>
    <b>cmake -S . -B build_debug -DCMAKE_MESSAGE_LOG_LEVEL=DEBUG -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON -DEnableDebugGlobal:BOOL=OFF</b>
    <b>cmake --build build_debug --config Debug --parallel 8</b>

Or to make a release build:

    <b>mkdir build_release</b>
    <b>cmake -S . -B build_release -DCMAKE_BUILD_TYPE=Release</b>
    <b>cmake --build build_release --config Release --parallel 8</b>

## Adding the ai-utils submodule to a project

To add this submodule to a project, that project should already
be set up to use [cwm4](https://github.com/CarloWood/cwm4).

Simply execute the following in a directory of that project
where you want to have the ``utils`` subdirectory (the
root of the project is recommended as that is the only thing
I've tested so far):

    git submodule add https://github.com/CarloWood/ai-utils.git utils

This should clone ai-utils into the subdirectory ``utils``, or
if you already cloned it there, it should add it.

### Using cmake

Check out the submodules [cwds](https://github.com/CarloWood/cwds) and [cwm4](https://github.com/CarloWood/cwm4) in the root of the project:

    git submodule add https://github.com/CarloWood/cwds.git
    git submodule add https://github.com/CarloWood/cwm4.git

The easiest way to use libcwd is by using [gitache](https://github.com/CarloWood/gitache).

For that to happen create in the root of the project (that uses utils)
a directory ``cmake/gitache-configs`` and put in it the file ``libcwd\_r.cmake``
with the content:

    gitache_config(
      GIT_REPOSITORY
        "https://github.com/CarloWood/libcwd.git"
      GIT_TAG
        "master"
      CMAKE_ARGS
        "-DEnableLibcwdAlloc:BOOL=OFF -DEnableLibcwdLocation:BOOL=ON"
   )

Add the variable ``GITACHE\_ROOT`` to your environment,
for example add to your ``~/.bashrc`` the line:

    export GITACHE_ROOT="/opt/gitache"

Add the following lines to the ``CMakeLists.txt`` in the
root of the project (directly under the ``project`` line):

    # Begin of gitache configuration.
    set(GITACHE_PACKAGES libcwd_r)
    include(cwm4/cmake/StableGitache)
    # End of gitache configuration.

    include(cwm4/cmake/AICxxProject)
    include(AICxxSubmodules)

``add\_subdirectory`` is not necessary for ``cwds``, ``cwm4`` or ``utils``.

See for example the root [MakeLists.txt](https://github.com/CarloWood/ai-utils-testsuite/blob/master/CMakeLists.txt) of of ai-utils-testsuite.

Finally, linking is done by adding ``${AICXX\_OBJECTS\_LIST}`` to
the appropriate ``target\_link\_libraries``.

For example,

    include(AICxxProject)

    add_executable(register_test register_test.cxx)
    target_link_libraries(register_test PRIVATE ${AICXX_OBJECTS_LIST})

See this [MakeLists.txt](https://github.com/CarloWood/ai-utils-testsuite/blob/master/src/CMakeLists.txt)
of ai-utils-testsuite for a complete example.

### Using GNU autotools

Changes to ``configure.ac`` and ``Makefile.am``
are taken care of by ``cwm4``, except for linking
which works as usual;

for example, a module that defines a

    bin_PROGRAMS = singlethreaded_foobar multithreaded_foobar

would also define

    singlethreaded_foobar_CXXFLAGS = @LIBCWD_FLAGS@
    singlethreaded_foobar_LDADD = ../utils/libutils.la $(top_builddir)/cwds/libcwds.la

    multithreaded_foobar_CXXFLAGS = @LIBCWD_R_FLAGS@
    multithreaded_foobar_LDADD = ../utils/libutils_r.la $(top_builddir)/cwds/libcwds_r.la

or whatever the path to ``utils`` is, to link with the required submodules,
libraries, and assuming you also use the [cwds](https://github.com/CarloWood/cwds) submodule.

Finally, run

    ./autogen.sh

to let cwm4 do its magic, and commit all the changes.

Checkout [ai-utils-testsuite](https://github.com/CarloWood/ai-utils-testsuite)
for an example of a project that uses this submodule.
