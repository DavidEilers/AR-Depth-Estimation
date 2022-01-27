project('openvr', 'cpp')

system_info = host_machine

lib_dir =''
bin_dir =''
if system_info.system() == 'windows'
  lib_dir='/lib/win'
  bin_dir='/bin/win'
elif system_info.system() == 'linux'
  lib_dir='/lib/linux'
  bin_dir='/bin/linux'
endif

if system_info.cpu_family() == 'x86' 
  lib_dir+='32'
  bin_dir+='32'
elif system_info.cpu_family() == 'x86_64'
  lib_dir+='64'
  bin_dir+='64'
endif
  
cc = meson.get_compiler('cpp', native: false)

openvr_lib = cc.find_library('openvr_api', dirs : meson.current_source_dir()+bin_dir, include_directories: [include_directories('headers')])
openvr_dep = declare_dependency(dependencies: openvr_lib,
                              include_directories:
                                    [include_directories('headers')])
openvr_inc = include_directories('headers')
openvr_bin_dir = meson.current_source_dir()+bin_dir