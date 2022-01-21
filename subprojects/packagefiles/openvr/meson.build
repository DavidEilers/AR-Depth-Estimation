project('openvr', 'cpp')

system_info = target_machine

lib_dir =''
bin_dir =''
if system_info.system() == 'windows'
  lib_dir='\\lib\\win'
  bin_dir='\\bin\\win'
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
  
cc = meson.get_compiler('cpp')

openvr_dep = cc.find_library('openvr_api', dirs : meson.current_source_dir()+lib_dir)
openvr_inc = include_directories('headers')
openvr_bin_dir = meson.current_source_dir()+bin_dir