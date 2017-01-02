m4_if(cwm4_rel_top_srcdir, [], [m4_append_uniq([CW_SUBMODULE_SUBDIRS], cwm4_submodule_path, [ ])])
m4_append_uniq([CW_SUBMODULE_CONFIG_FILES], cwm4_quote(cwm4_submodule_relpath[Makefile]), [ ])
