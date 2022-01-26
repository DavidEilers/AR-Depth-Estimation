project('glad','c',default_options:['warning_level=0'])

src = 'src/gl.c'

#glad_library = shared_library('glad',src, include_directories: include_directories('include'))
glad_dep = declare_dependency(include_directories: include_directories('include'), sources:src)# link_with: glad_library, include_directories: include_directories('include'))