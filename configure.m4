m4_if(cwm4_submodule_dirname, [], [m4_append_uniq([CW_SUBMODULE_SUBDIRS], cwm4_submodule_basename, [ ])])
m4_append_uniq([CW_SUBMODULE_CONFIG_FILES], cwm4_quote(cwm4_submodule_path[/Makefile]), [ ])

# Add configuration options for utils.

AC_ARG_ENABLE(debug-global,
    [  --disable-debug-global  disable debug code that checks the correct usage of Global<> and Singleton<>.],
    cw_config_debug_global=$enableval, cw_config_debug_global=$cw_config_debug)

AC_SUBST(CW_CONFIG_DEBUGGLOBAL)
CW_CONFIG_DEBUGGLOBAL=undef
CW_EXTRA_CWDS_LIBS=
CW_EXTRA_CWDS_R_LIBS=

if test "$cw_config_debug_global" = yes; then
  CW_CONFIG_DEBUGGLOBAL=define
  CW_EXTRA_CWDS_LIBS="../utils/libutils.la"
  CW_EXTRA_CWDS_R_LIBS="../utils/libutils_r.la"
fi

AC_ARG_ENABLE(debug-llists,
    [  --enable-debug-llists   include debug code that checks for the correct usage of the linked lists.],
    cw_config_debug_llists=$enableval, cw_config_debug_llists=no)

AC_SUBST(CW_CONFIG_DEBUGLLISTS)
CW_CONFIG_DEBUGLLISTS=undef

if test "$cw_config_debug_llists" = yes; then
  CW_CONFIG_DEBUGLLISTS=define
fi

m4_append_uniq([CW_SUBMODULE_CONFIG_FILES], cwm4_quote(cwm4_submodule_path[/config.h]), [ ])

if test "$cw_config_debug" != "yes"; then
  if test "$cw_config_debug_global" = yes; then
    AC_MSG_ERROR([

You can not specify --enable-debug-global without including debugging
code (--enable-debug).
])
  fi
fi

# Test if we have the __builtin_expect builtin.
CW_HAVE_BUILTIN_EXPECT

AH_BOTTOM([#include "utils/config.h"])
