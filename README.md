# ai-utils submodule

This repository is a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
providing C++ utilities for larger projects, including:

* ``AIAlert`` : an exception based error reporting system.
* ``AIFIFOBuffer`` : A spsc lock-free ring buffer for trivially copyable objects.
* ``AIRefCount`` : Base class for classes that need to wrapped into as ``boost::intrusive_ptr``.
* ``Array`` / ``Vector`` / ``Deque`` : A wrapper around ``std::array`` / ``std::vector`` / ``std::deque`` that only allow a specific type as index.
* ``AtomicFuzzyBool`` / ``FuzzyBool`` : Fuzzy booleans; great for conditions that are subject to races in a multi-threaded application.
* ``Badge`` : No need to make a class a friend in order to access ONE member function! Just give it access to that one member function.
* ``BitSet<T>`` : A wrapper around unsigned integral types T that allows fast bit-level manipulation, including iterating in a loop over all set bits.
* ``ColorPool`` : Allows to hand out a "color" (just a small int, an index), from a pool, that wasn't used for the longest period. Intended to color debug output of threads and used by [threadpool](https://github.com/CarloWood/threadpool).
* ``DelayLoopCalibration`` : Determine the required loop size for a given lambda to delay the code a given amount of milliseconds.
* ``DequeAllocator`` : The perfect allocator for your deque's.
* ``DEVector`` : Double-Ended Vector. Contiguous storage, but sacrifices memory to be as cheap with push_front as push_back.
* ``Dictionary`` : Map known words to known enum values, and unknown words to new (different) values.
* ``EnumIterator`` : Iterate over (contiguous) enum values.
* ``FunctionView`` : Cheap, lightweight Callable (like std::function) suitable for passing arbitrary functions as argument.
* ``Global`` / ``Singleton`` : template classes for global objects.
* ``InsertExtraInitialization`` : add extra initialization between base class constructor and constructor.
* ``InstanceTracker`` : Base class to keep track of all existing objects of a given type.
* ``iomanip`` : Custom io manipulators.
* ``itoa`` : Maximum speed integer to string converter.
* ``List`` : Drop-in for std::list - slightly faster - and with extra features: you can test if an iterator is_end() or is_begin() without access to the list, and you can go to the next previous element, having a value_type, without access to the list (next(), prev()).
* ``MemoryPagePool`` : A memory pool that returns fixed-size memory blocks allocated with ``std::aligned_alloc`` and aligned to ``memory_page_size``.
* ``MultiLoop`` : A variable number of nested for loops.
* ``NodeMemoryPool`` : A memory pool intended for fixed size allocations, one object at a time, where the size and type of the object are not known until the first allocation. Intended to be used with ``std::allocate_shared`` or ``std::list``.
* ``NodeMemoryResource`` : A fixed size memory resource that uses a ``MemoryPagePool`` as upstream.
* ``ObjectTracker`` : Keeps track of where your object is (even if it is moved). Can be used as a pointer to objects that might move in memory.
* ``pointer_hash`` : The ideal hash function for pointers returned by new or malloc (or any pointer really).
* ``PairCompare`` : Function to compare two pairs pair<First, Second>. First and Second are allowed to be vectors.
* ``QuotedList`` : Use together with utils::print_using to print elements of a container.
* ``RandomNumber`` : Wrapper around a std::mt19937_64 to more intuitively generate uniformly distributed integers.
* ``RandomStream`` : Stream producing random characters.
* ``Register`` : Register callbacks for global objects, to be called once main() is entered.
* ``REMOVE_TRAILING_COMMA`` : Macro that removes the last (possibly empty) argument.
* ``SimpleSegregatedStorage`` : Maintains an unordered free list of blocks (used by NodeMemoryResource and MemoryPagePool).
* ``Signals`` : Finally get your POSIX signals working the Right Way(tm).
* ``StreamHasher`` : Calculate a digest of input written using operator<<.
* ``TemplateStringLiteral`` : Pass a string literal as template parameter.
* ``u8string_to_filename`` : convert any UTF8 string to a still human readable and legal filename - and back if you want.
* ``UltraHash`` : convert 64-bit keys into a small lookup table index [0..256] in 67 clock cycles.
* ``UniqueID.h`` : Hands out unique IDs, unique within a given context.
* ``VectorCompare`` : Functor to compare two vectors.
* ``VTPtr`` : Custom virtual table for classes. The advantage being that the virtual table is dynamic and can be altered during runtime.

* Several utilities like
* ``almost_equal`` : compare two floating point types.
* ``at_scope_end`` : exception safe execute code when the current scope is left.
* ``c_escape`` : write char range (string, string_view, vector<char>) to an ostream, escaping unprintable characters (e.g. \n, \e, \0, \xAC).
* ``clz / ctz / mssb / parity / popcount`` : wrappers around __builtin functions.
* ``concat_array`` : constexpr catenate std::array's.
* ``constexpr_ceil``: constexpr ceil function.
* ``cpu_relax`` : for your spin loops.
* ``derived_from_template`` : type trait to determine if it is derived from a certain template.
* ``double_to_str_precision`` : convert a double to a std::string with given precision.
* ``for_each_until`` : run function on elements until the function returns true; returns true iff such an element exists.
* ``generate_unique_filename`` : return non-existing filename from hint (e.g. "foo.txt --> foo (1).txt" if foo.txt already exists).
* ``get_Nth_type`` : return the Nth type of a template parameter pack.
* ``has_print_on`` : never write ostream operator<< again! Use a member function to print a class.
* ``is_complete`` : test if a template parameter was already completely defined.
* ``is_pointer_like`` : type traits to detect if a type can be derefenced like a pointer.
* ``is_power_of_two`` : (constexpr) test if integral value is a power of two.
* ``is_specialization_of`` : type trait to test if a type is a specialization of a given template type.
* ``is_vector`` : type trait to test if a type is std::vector or utils::Vector.
* ``log2 / ceil_log2`` : (constexpr) return floor or ceil log2 of integral value.
* ``malloc_size`` : calculate the optimal size to allocate, given a required minimum.
* ``nearest_multiple_of_power_of_two`` : returns the smallest possible value N * power_of_two that is greater than or equal to n.
* ``nearest_power_of_two`` : round the positive integer n up to the nearest power of 2.
* ``print_using`` : insert into an ostream, specifying how to print the type (e.g cout << utils::print_using(obj, <how to print it>) << ...).
* ``pointer_hash`` : suited to calculate a 64-bit hash from two or more 64-bit pointers to (heap allocated) memory.
* ``reverse_bits`` : (constexpr) reverse the bits of integral value.
* ``reversed`` : non-ranges adaptor to reverse a container.
* ``sorted_vector_insert`` : insert a new element into an already sorted vector.
* ``split`` : call a lambda on every token in a string (separated by a given char delimiter).
* ``square`` : because it just looks more sane than x * x.
* ``three_way_merge`` : the reason std::three_way_merge still doesn't exist is because it is above most peoples pay grade.
* ``type_id_hash`` : consteval type id hash (uint64_t).
* ``ulong_to_base`` : convert ulong to base 26.
* ``unstable_remove`` : for fast removal of elements from a vector.
* ``utf8_glyph_length`` : return length of multichar char8_t glyph.
* ``macros.h`` : convenience macros.

The root project should be using
[cmake](https://cmake.org/overview/)
[cwm4](https://github.com/CarloWood/cwm4) and
[cwds](https://github.com/CarloWood/cwds).

## Checking out a project that uses the ai-utils submodule.

To clone a project example-project that uses ai-utils simply run:

    git clone --recursive <URL-to-project>/example-project.git
    cd example-project
    AUTOGEN_CMAKE_ONLY=1 ./autogen.sh

The ``--recursive`` is optional because ``./autogen.sh`` will fix
it when you forgot it.

When using [GNU autotools](https://en.wikipedia.org/wiki/GNU_Autotools) you should of course
not set ``AUTOGEN_CMAKE_ONLY``. Also, you probably want to use ``--enable-mainainer-mode``
as option to the generated ``configure`` script. ***WARNING: autotools are no longer tested (supported) by the author***

In order to use ``cmake`` configure as usual, for example to do a debug build with 16 cores:

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
