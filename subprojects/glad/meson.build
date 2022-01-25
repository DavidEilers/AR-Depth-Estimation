project('glad','c')

src = 'src/gl.c'

glad_library = shared_library('glad',src, include_directories: include_directories('include'))
glad_dep = declare_dependency(link_with: glad_library, include_directories: include_directories('include'))