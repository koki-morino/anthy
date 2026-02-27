libsplit_sources = [
  'wordlist.c',
  'metaword.c',
  'depgraph.c',
  'splitter.c',
  'evalborder.c',
  'compose.c',
  'lattice.c',
  'segclass.c',
]

libsplit = static_library('split',
  sources: libsplit_sources,
  include_directories: [top_srcdir, win32dir],
  dependencies: [cc.find_library('m', required: false)]
)

split_dep = declare_dependency(link_with: libsplit)
