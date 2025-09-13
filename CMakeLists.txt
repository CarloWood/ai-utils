cmake_minimum_required(VERSION 3.12...3.26)

project(utils
  DESCRIPTION "C++ Core utilities"
  LANGUAGES CXX C       # Language "C" is required for find_package(Threads).
)

include(AICxxProject)

#==============================================================================
# POLICIES
#
if (POLICY CMP0167)
  # Use boost provided BoostConfig.cmake (requires boost version 1.70 or higher).
  # This policy was introduced in cmake 3.30.
  cmake_policy(SET CMP0167 NEW)
endif ()

#==============================================================================
# DEPENDENCIES
#

# AICxx projects are always multi-threaded.
# As utils is also always required, this is the last place where Threads need to be mentioned.
find_package(Threads REQUIRED)

# This project uses header-only boost libraries:
# AIAlert.h:      #include <boost/lexical_cast.hpp>
# AIRefCount.h:   #include <boost/intrusive_ptr.hpp>
# macros.h:       #include <boost/preprocessor/stringize.hpp>
#                 #include <boost/preprocessor/expand.hpp>
# StreamHasher.h: #include <boost/functional/hash.hpp>
find_package(Boost CONFIG REQUIRED)

# Test if the root of the project used `find_package(Boost CONFIG COMPONENTS filesystem)`.
if (Boost_FILESYSTEM_FOUND)
  set(HAVE_BOOST_FILESYSTEM 1)
endif ()

#==============================================================================
# OPTIONS

# Detect if libcwd was found and if it was configured with CWDEBUG_ALLOC and CWDEBUG_LOCATION.
if (libcwd_r_ROOT AND EXISTS "${libcwd_r_ROOT}/include/libcwd/config.h")
  file(WRITE "${CMAKE_BINARY_DIR}/test.cpp"
    "#include \"include/libcwd/config.h\"\n"
    "#if CWDEBUG_ALLOC && CWDEBUG_LOCATION\n"
    "True\n"
    "#else\n"
    "False\n"
    "#endif\n"
  )
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -I "${libcwd_r_ROOT}" -E "${CMAKE_BINARY_DIR}/test.cpp"
    OUTPUT_VARIABLE PREPROCESSOR_OUTPUT
  )
  string(REGEX MATCH "\n(True|False)\n" LIBCWD_R_CONFIG_RESULT ${PREPROCESSOR_OUTPUT})
  string(REGEX REPLACE "\n" "" REPLACE_OUTPUT ${LIBCWD_R_CONFIG_RESULT})
  message(DEBUG "${libcwd_r_ROOT}/include/libcwd/config.h has both CWDEBUG_ALLOC and CWDEBUG_LOCATION set: ${REPLACE_OUTPUT}")
  set(libcwd_r_CWDEBUG_ALLOC_and_CWDEBUG_LOCATION_found ${REPLACE_OUTPUT})
  unset(REPLACE_OUTPUT)
  unset(PREPROCESSOR_OUTPUT)
  unset(LIBCWD_R_CONFIG_RESULT)
else()
  set(libcwd_r_CWDEBUG_ALLOC_and_CWDEBUG_LOCATION_found False)
endif()

# Option 'EnableDebugGlobal' is available when OptionEnableLibcwd is ON (otherwise OFF).
cw_option(EnableDebugGlobal
          "Enable debug code that checks the correct usage of Global<> and Singleton<>" ON
          "OptionEnableLibcwd;libcwd_r_CWDEBUG_ALLOC_and_CWDEBUG_LOCATION_found" OFF
)

if (OptionEnableLibcwd)
  message(DEBUG "OptionEnableDebugGlobal is ${OptionEnableDebugGlobal}")
endif ()
if (OptionEnableDebugGlobal)
  set(DEBUGGLOBAL 1)
endif ()

# Only check for enchantum if we're using at least C++20.
if (DEFINED CMAKE_CXX_STANDARD AND CMAKE_CXX_STANDARD GREATER_EQUAL 20)
  # Do we have an enchantum submodule?
  # To use this, add https://github.com/CarloWood/enchantum.git as git submodule to the root of the project
  # and add `add_subdirectory(enchantum)` to the root CMakeLists.txt file.
  if (EXISTS "${CMAKE_SOURCE_DIR}/enchantum/enchantum/include/enchantum/enchantum.hpp")
    set(OptionUseEnchantum True)
    message(STATUS "Enchantum enabled")
  else ()
    message(STATUS "Enchantum submodule not found: enchantum disabled")
  endif ()
endif ()

#==============================================================================
# PLATFORM SPECIFIC CHECKS
#

include(CW_CHECK_REQUIRED_INCLUDE_FILE_CXX)
if (OptionEnableDebugGlobal)
  CW_CHECK_REQUIRED_INCLUDE_FILE_CXX("execinfo.h"
    "Sorry, Option EnableDebugGlobal requires execinfo.h for the backtrace(3) function.\nPlease specify -DEnableDebugGlobal:BOOL=OFF as configure option or fix this problem.")
endif ()

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("int main() { return __builtin_expect(0, 1); }" HAVE_BUILTIN_EXPECT)

include(CW_SYS_MALLOC_OVERHEAD)
CW_SYS_MALLOC_OVERHEAD()                # Determine the overhead in bytes of an allocation with malloc(2).

include(CW_SYS_CACHELINE_SIZE)
CW_SYS_CACHELINE_SIZE()                 # Find the cacheline size.

#include(CW_CHECK_BMI2_SUPPORT)
#CW_CHECK_BMI2_SUPPORT()                 # Test for bmi2 support.
#set(GCC_BMI2_COMPILE_FLAGS "-mbmi2")    # Will be added if support was detected, see below.

if (OptionUseEnchantum)
  set(USE_ENCHANTUM 1)
endif ()

# Specify utils specific configure file.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h
               @ONLY)

# Clean up rule for the above.
add_custom_target(maintainer-clean-utils
  #  COMMAND rm config.h        # Can't remove this one with a custom command: it is re-generated :/
  COMMAND rm -rf cw_utils_sys_malloc_overhead cw_utils_sys_cacheline_size
)

#==============================================================================
# BUILD OBJECT LIBRARY
#

# This project is an OBJECT-library, used by other git submodules and the main project.
add_library(utils_ObjLib OBJECT)

# Tell cwds/sys.h that we have a config.h.
target_compile_definitions(utils_ObjLib
  PUBLIC
    HAVE_UTILS_CONFIG_H
)

# The list of source files.
target_sources(utils_ObjLib
  PRIVATE
    "AIAlert.cxx"
    "DelayLoopCalibration.cxx"
    "Dictionary.cxx"
    "FuzzyBool.cxx"
    "GlobalObjectManager.cxx"
    "RandomNumber.cxx"
    "Register.cxx"
    "Signals.cxx"
    "UltraHash.cxx"

    "debug_ostream_operators.cxx"
    "double_to_str_precision.cxx"
    "itoa.cxx"
    "malloc_size.cxx"
    "print_using.cxx"
    "translate.cxx"
    "u8string_to_filename.cxx"
    "utf8_glyph_length.cxx"
    "to_digits_string.cxx"

    "threading/aithreadid.cxx"
    "threading/Semaphore.cxx"
    "threading/SpinSemaphore.cxx"

    "AIAlert.h"
    "AIRefCount.h"
    "DelayLoopCalibration.h"
    "Dictionary.h"
    "FunctionView.h"
    "FuzzyBool.h"
    "Global.h"
    "GlobalObjectManager.h"
    "MultiLoop.h"
    "ObjectTracker.h"
    "Register.h"
    "Signals.h"
    "Singleton.h"
    "UltraHash.h"

    "apply_function.h"
    "at_scope_end.h"
    "cpu_relax.h"
    "debug_ostream_operators.h"
    "double_to_str_precision.h"
    "for_each_until.h"
    "is_pointer_like.h"
    "is_power_of_two.h"
    "itoa.h"
    "log2.h"
    "macros.h"
    "malloc_size.h"
    "print_using.h"
    "translate.h"
    "u8string_to_filename.h"
    "ulong_to_base.h"
    "utf8_glyph_length.h"
    "to_digits_string.h"

    "threading/aithreadid.h"
    "threading/FIFOBuffer.h"
    "threading/Futex.h"
    "threading/Gate.h"
    "threading/MpscQueue.h"
    "threading/Semaphore.h"
    "threading/SpinSemaphore.h"
    "threading/StartingGate.h"
)

if (EXISTS "${CMAKE_SOURCE_DIR}/threadsafe")
target_sources(utils_ObjLib
  PRIVATE
    "InstanceTracker.cxx"
    "InstanceTracker.h"
)
endif ()

if (Boost_FILESYSTEM_FOUND)
target_sources(utils_ObjLib
  PRIVATE
    "generate_unique_filename.cxx"
)

target_link_libraries(utils_ObjLib
  PUBLIC
    Boost::filesystem
)
endif ()

if (OptionUseEnchantum)
target_link_libraries(utils_ObjLib
  INTERFACE
    enchantum::enchantum
)
endif ()

# Required include search-paths.
get_target_property(CWDS_INTERFACE_INCLUDE_DIRECTORIES AICxx::cwds INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(utils_ObjLib
  PUBLIC
    "${CWDS_INTERFACE_INCLUDE_DIRECTORIES}" # For sys.h and debug.h.
    "${top_objdir}"                         # For utils/config.h.
)

# Require support for C++20.
target_compile_features(utils_ObjLib
  PUBLIC cxx_std_20
)

# Add GCC_BMI2_COMPILE_FLAGS if supported.
#if (CW_BMI2_SUPPORT)
#  get_target_property(TMP_COMPILE_FLAGS utils_ObjLib COMPILE_FLAGS)
#  if (TMP_COMPILE_FLAGS STREQUAL "TMP_COMPILE_FLAGS-NOTFOUND")
#    set(TMP_COMPILE_FLAGS "")
#  else ()
#    set(TMP_COMPILE_FLAGS "${TMP_COMPILE_FLAGS} ")
#  endif ()
#  set(TMP_COMPILE_FLAGS "${TMP_COMPILE_FLAGS}${GCC_BMI2_COMPILE_FLAGS}")
#  set_target_properties(utils_ObjLib PROPERTIES COMPILE_FLAGS ${TMP_COMPILE_FLAGS})
#endif ()

# Set link dependencies.
target_link_libraries(utils_ObjLib
  PUBLIC
    Boost::boost
    Threads::Threads
)

if (OptionEnableLibcwd)
  target_link_libraries(utils_ObjLib
    PUBLIC
      ${libcwd_r_TARGET}
)
endif ()

# Create an ALIAS target.
add_library(AICxx::utils ALIAS utils_ObjLib)

# Prepend this object library to the list.
set(AICXX_OBJECTS_LIST AICxx::utils ${AICXX_OBJECTS_LIST} CACHE INTERNAL "List of OBJECT libaries that this project uses.")
