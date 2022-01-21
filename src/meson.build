include_dirs = []

cc = meson.get_compiler('cpp')

### Get the neccessary dependencies for OpenVR
openvr_proj = subproject('openvr')
openvr_dep  = openvr_proj.get_variable('openvr_dep')
openvr_inc = openvr_proj.get_variable('openvr_inc')
include_dirs += openvr_inc

## Workaround on Windows to copy openvr_api.dll, because it won't link with meson dependencies 
if target_machine.system() == 'windows'
 copy_cmd = find_program('test.bat')
 openvr_dll_dir = openvr_proj.get_variable('openvr_bin_dir')
 run_command(copy_cmd,openvr_dll_dir+'\\openvr_api.dll',meson.current_build_dir()+'\\openvr_api.dll', check: true)
endif


##### main program #####
src = []
main_src = src + 'main.cpp'
main_exe = executable('main', main_src, include_directories: include_dirs)


##### testing part #####

### Get the necessary dependencvies for Google Test 
gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')

### List all test-files in a array and create a seperate Meson test for every test-file
test_src = src + ['image_test.cpp','init_vr_test.cpp']
foreach single_test : test_src
    temp_exe = executable(single_test.strip('.cpp'),[single_test]+'test_main.cpp', dependencies:[gtest_dep,openvr_dep], include_directories: include_dirs)
    test(single_test.strip('_test.cpp'),temp_exe)
endforeach

### Create a Test which includes all other tests
test_src += 'test_main.cpp'
test_exe = executable('all_tests',test_src, dependencies:[gtest_dep,openvr_dep], include_directories: include_dirs)
test('all_tests', test_exe)