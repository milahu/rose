include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

AM_CPPFLAGS = $(ROSE_INCLUDES)

noinst_LTLIBRARIES = libRoseAST.la

libRoseAST_la_SOURCES = \
  graphviz.C \
  io/merge.C io/link.C io/delete.C io/share.C io/load.C \
  checker/cmdline.C checker/checker.C \
  checker/integrity_edges.C checker/integrity_declarations.C checker/integrity_symbols.C checker/integrity_types.C \
  utils/edge_ptr_repl.C
libRoseAST_la_LIBADD = 

libRoseAST_la_public_includedir = $(prefix)/include/Rose
libRoseAST_la_public_include_HEADERS = ../AST.h

libRoseAST_la_includedir = $(prefix)/include/Rose/AST
libRoseAST_la_include_HEADERS = cmdline.h IO.h Checker.h Defect.h Utils.h


EXTRA_DIST = CMakeLists.txt

