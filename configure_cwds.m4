# cwds depends on utils if --disable-global-debug is not specified.
m4_if(cwm4_submodule_dirname, [], [m4_append_uniq([CW_SUBMODULE_SUBDIRS], utils, [ ])])
