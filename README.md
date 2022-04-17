# ai-utils submodule

This repository is a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
providing C++ utilities for larger projects, including:

* ``AIAlert`` : an exception based error reporting system.
* ``AIFIFOBuffer`` : A spsc lock-free ring buffer for trivially copyable objects.
* ``AIRefCount`` : Base class for classes that need to wrapped into as ``boost::intrusive_ptr``.
* ``AISignals`` : C++ wrapper around POSIX signals.
* ``Array`` / ``Vector`` : A wrapper around ``std::array`` / ``std::vector`` that only allow a specific type as index.
* ``AtomicFuzzyBool`` / ``FuzzyBool`` : Fuzzy booleans; great for conditions that are subject to races in a multi-threaded application.
* ``Badge`` : No need to make a class a friend in order to access ONE member function! Just give it access to that one member function.
* ``BitSet<T>`` : A wrapper around unsigned integral types T that allows fast bit-level manipulation, including iterating in a loop over all set bits.
* ``ColorPool`` : Allows to hand out a "color" (just a small int, an index), from a pool, that wasn't used for the longest period. Intended to color debug output of threads and used by [threadpool](https://github.com/CarloWood/threadpool).
* ``DelayLoopCalibration`` : Determine the required loop size for a given lambda to delay the code a given amount of milliseconds.
* ``DequeAllocator`` : The perfect allocator for your deque's.
* ``Dictionary`` : Map known words to known enum values, and unknown words to new (different) values.
* ``FunctionView`` : Cheap, lightweight Callable (like std::function) suitable for passing arbitrary functions as argument.
* ``Global`` / ``Singleton`` : template classes for global objects.
* ``InstanceTracker`` : Base class to keep track of all existing objects of a given type.
* ``iomanip`` : Custom io manipulators.
* ``itoa`` : Maximum speed integer to string converter.
* ``MemoryPagePool`` : A memory pool that returns fixed-size memory blocks allocated with ``std::aligned_alloc`` and aligned to ``memory_page_size``.
* ``MultiLoop`` : A variable number of nested for loops.
* ``NodeMemoryPool`` : A memory pool intended for fixed size allocations, one object at a time, where the size and type of the object are not known until the first allocation. Intended to be used with ``std::allocate_shared`` or ``std::list``.
* ``NodeMemoryResource`` : A fixed size memory resource that uses a ``MemoryPagePool`` as upstream.
* ``pointer_hash`` : The ideal hash function for pointers returned by new or malloc (or any pointer really).
* ``RandomStream`` : Stream producing random characters.
* ``Register`` : Register callbacks for global objects, to be called once main() is entered.
* ``REMOVE_TRAILING_COMMA`` : Macro that removes the last (possibly empty) argument.
* ``SimpleSegregatedStorage`` : Maintains an unordered free list of blocks (used by NodeMemoryResource and MemoryPagePool).
* ``Signals`` : Finally get your POSIX signals working the Right Way(tm).
* ``StreamHasher`` : Calculate a digest of input written using operator<<.
* ``u8string_to_filename`` : convert any UTF8 string to a still human readable and legal filename - and back if you want.
* ``UltraHash`` : convert 64-bit keys into a small lookup table index [0..256] in 67 clock cycles.
* ``UniqueID.h`` : Hands out unique IDs, unique within a given context.
* ``VTPtr`` : Custom virtual table for classes. The advantage being that the virtual table is dynamic and can be altered during runtime.

* Several utilities like ``almost_equal``, ``at_scope_end``, ``c_escape``, ``clz / ctz / mssb / parity / popcount``,
  ``constexpr_ceil``, ``cpu_relax``, ``double_to_str_precision``, ``for_each_until``, ``get_Nth_type``,
  ``is_pointer_like``, ``is_power_of_two``, ``log2``, ``malloc_size``,
  ``nearest_multiple_of_power_of_two``, ``nearest_power_of_two``, ``print_using`` ``reversed``,
  ``split``, ``ulong_to_base``, ``unstable_remove`` and convenience macros.

The root project should be using
[cmake](https://cmake.org/overview/)
[cwm4](https://github.com/CarloWood/cwm4) and
[libcwd](https://github.com/CarloWood/libcwd).

## Checking out a project that uses the ai-utils submodule.

To clone a project example-project that uses ai-utils simply run:

    git clone --recursive <URL-to-project>/example-project.git
    cd example-project
    AUTOGEN_CMAKE_ONLY=1 ./autogen.sh

The ``--recursive`` is optional because ``./autogen.sh`` will fix
it when you forgot it.

When using [GNU autotools](https://en.wikipedia.org/wiki/GNU_Autotools) you should of course
not set ``AUTOGEN_CMAKE_ONLY``. Also, you probably want to use ``--enable-mainainer-mode``
as option to the generated ``configure`` script.

In order to use ``cmake`` configure as usual, for example to build with 8 cores a debug build:

    mkdir build_debug
    cmake -S . -B build_debug -DCMAKE_MESSAGE_LOG_LEVEL=DEBUG -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON -DEnableDebugGlobal:BOOL=OFF
    cmake --build build_debug --config Debug --parallel 16

Or to make a release build:

    mkdir build_release
    cmake -S . -B build_release -DCMAKE_BUILD_TYPE=Release
    cmake --build build_release --config Release --parallel 16

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
a directory ``cmake/gitache-configs`` and put in it the file ``libcwd_r.cmake``
with the content:

    gitache_config(
      GIT_REPOSITORY
        "https://github.com/CarloWood/libcwd.git"
      GIT_TAG
        "master"
      CMAKE_ARGS
        "-DEnableLibcwdAlloc:BOOL=OFF -DEnableLibcwdLocation:BOOL=ON"
   )

Add the variable ``GITACHE_ROOT`` to your environment,
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

``add_subdirectory`` is not necessary for ``cwds``, ``cwm4`` or ``utils``.

See for example the root [MakeLists.txt](https://github.com/CarloWood/ai-utils-testsuite/blob/master/CMakeLists.txt) of ai-utils-testsuite.

Finally, linking is done by adding ``${AICXX_OBJECTS_LIST}`` to
the appropriate ``target_link_libraries``.

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
