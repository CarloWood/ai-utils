cmake_minimum_required( VERSION 3.12...3.16 )

project( utils
         DESCRIPTION "C++ Core utilities"
         LANGUAGES CXX C        # Language "C" is required for find_package(Threads).
)

include( AICxxProject )

#==============================================================================
# DEPENDENCIES
#

# AICxx projects are always multi-threaded.
# As utils is also always required, this is the last place where Threads need to be mentioned.
find_package( Threads REQUIRED )

# This project uses header-only boost libraries:
# AIAlert.h:      #include <boost/lexical_cast.hpp>
# AIRefCount.h:   #include <boost/intrusive_ptr.hpp>
# macros.h:       #include <boost/preprocessor/stringize.hpp>
#                 #include <boost/preprocessor/expand.hpp>
# StreamHasher.h: #include <boost/functional/hash.hpp>
find_package( Boost REQUIRED )

#==============================================================================
# OPTIONS

# Option 'EnableDebugGlobal' is available when OptionEnableLibcwd is ON (otherwise OFF).
cw_option( EnableDebugGlobal
        "Enable debug code that checks the correct usage of Global<> and Singleton<>" ON
        "OptionEnableLibcwd" OFF )

if ( OptionEnableLibcwd )
  message( DEBUG "OptionEnableDebugGlobal is ${OptionEnableDebugGlobal}" )
endif ()
if ( OptionEnableDebugGlobal )
  set( DEBUGGLOBAL 1 )
endif ()

#==============================================================================
# PLATFORM SPECIFIC CHECKS
#

include( CW_CHECK_REQUIRED_INCLUDE_FILE_CXX )
CW_CHECK_REQUIRED_INCLUDE_FILE_CXX( "sparsehash/dense_hash_map" "Please install the libsparsehash-dev package." )

if ( OptionEnableDebugGlobal )
  CW_CHECK_REQUIRED_INCLUDE_FILE_CXX( "execinfo.h"
    "Sorry, Option EnableDebugGlobal requires execinfo.h for the backtrace(3) function.\nPlease specify -DEnableDebugGlobal:BOOL=OFF as configure option or fix this problem." )
endif ()

include( CheckCXXSourceCompiles )
check_cxx_source_compiles( "int main(void) { return __builtin_expect(0, 1); }" HAVE_BUILTIN_EXPECT )

include( CW_SYS_MALLOC_OVERHEAD )
CW_SYS_MALLOC_OVERHEAD()                # Determine the overhead in bytes of an allocation with malloc(2).

include( CW_SYS_CACHELINE_SIZE )
CW_SYS_CACHELINE_SIZE()                 # Find the cacheline size.

# Specify utils specific configure file.
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/config.h
                @ONLY )

# Clean up rule for the above.
add_custom_target( maintainer-clean-utils
    COMMAND rm config.h
    COMMAND rm -rf cw_utils_sys_malloc_overhead cw_utils_sys_cacheline_size
)

#==============================================================================
# BUILD OBJECT LIBRARY
#

# This project is an OBJECT-library, used by other git submodules and the main project.
add_library( utils_ObjLib OBJECT )

# Tell cwds/sys.h that we have a config.h.
target_compile_definitions( utils_ObjLib
    PUBLIC
        HAVE_UTILS_CONFIG_H
)

# Set SOURCE_DIR to the current directory.
set_property(
    TARGET     utils_ObjLib
    PROPERTY   SOURCE_DIR       "${CMAKE_CURRENT_SOURCE_DIR}"
)

# The list of source files.
target_sources( utils_ObjLib
    PRIVATE
        "AIAlert.cxx"
        "AISignals.cxx"
        "debug_ostream_operators.cxx"
        "DelayLoopCalibration.cxx"
        "double_to_str_precision.cxx"
        "FuzzyBool.cxx"
        "GlobalObjectManager.cxx"
        "itoa.cxx"
        "NodeMemoryPool.cxx"
        "malloc_size.cxx"
        "MemoryPagePool.cxx"
        "print_using.cxx"
        "translate.cxx"

        "AIAlert.h"
        "AIFIFOBuffer.h"
        "AIRefCount.h"
        "AISignals.h"
        "apply_function.h"
        "at_scope_end.h"
        "cpu_relax.h"
        "debug_ostream_operators.h"
        "DelayLoopCalibration.h"
        "double_to_str_precision.h"
        "for_each_until.h"
        "FunctionView.h"
        "FuzzyBool.h"
        "Global.h"
        "GlobalObjectManager.h"
        "is_power_of_two.h"
        "is_pointer_like.h"
        "itoa.h"
        "log2.h"
        "macros.h"
        "malloc_size.h"
        "MultiLoop.h"
        "MemoryPagePool.h"
        "NodeMemoryPool.h"
        "NodeMemoryResource.h"
        "print_using.h"
        "SimpleSegregatedStorage.h"
        "Singleton.h"
        "translate.h"
        "ulong_to_base.h"
)

# Required include search-paths.
get_target_property( CWDS_INTERFACE_INCLUDE_DIRECTORIES AICxx::cwds INTERFACE_INCLUDE_DIRECTORIES )
target_include_directories( utils_ObjLib
    PUBLIC
        "${CWDS_INTERFACE_INCLUDE_DIRECTORIES}" # For sys.h and debug.h.
        "${top_objdir}"                         # For utils/config.h.
)

# Require support for C++17.
target_compile_features( utils_ObjLib
    PUBLIC cxx_std_17
)

# Set link dependencies.
target_link_libraries( utils_ObjLib
    PUBLIC
        Boost::boost
        Threads::Threads
)

if ( OptionEnableLibcwd )
  target_link_libraries( utils_ObjLib
      PUBLIC
          PkgConfig::Libcwd_r
)
endif ()

# Create an ALIAS target.
add_library( AICxx::utils ALIAS utils_ObjLib )

# Prepend this object library to the list.
set( AICXX_OBJECTS_LIST AICxx::utils ${AICXX_OBJECTS_LIST} CACHE INTERNAL "List of OBJECT libaries that this project uses." )
