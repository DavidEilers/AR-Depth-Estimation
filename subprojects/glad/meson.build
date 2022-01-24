project('glad','c')

if build_machine.system() == 'windows'
    src = 'src\\gl.c'
else
    src = 'src/gl.c'
endif
glad_library = shared_library('glad',src, include_directories: include_directories('include'))
glad_dep = declare_dependency(link_with: glad_library, include_directories: include_directories('include'))