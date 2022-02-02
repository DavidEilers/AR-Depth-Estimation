include_dirs = []
dependencies = []

cc = meson.get_compiler('cpp', native: false)

### Get the neccessary dependencies for OpenVR
openvr_proj = subproject('openvr')
openvr_dep  = openvr_proj.get_variable('openvr_dep')
dependencies += [openvr_dep]

## Workaround on Windows to copy openvr_api.dll, because it won't link with meson dependencies 
if host_machine.system() == 'windows'
 copy_cmd = find_program('test.sh')
 openvr_dll_dir = openvr_proj.get_variable('openvr_bin_dir')
 run_command(copy_cmd,openvr_dll_dir+'/openvr_api.dll',meson.current_build_dir()+'/openvr_api.dll', check: true)
endif


dl_dep = cc.find_library('dl', required: false)
dependencies += [dl_dep]

### Get the neccessary dependencies for GLFW
glfw_proj = subproject('glfw')
glfw_dep = glfw_proj.get_variable('glfw_dep')
dependencies += [glfw_dep]

### Get the neccessary dependencies for OpenGL
opengl_dep = dependency('gl',required:true)
dependencies += [opengl_dep]

### Get the neccessary dependencies for STB
stb_proj = subproject('stb')
stb_dep = stb_proj.get_variable('stb_dep')
dependencies += [stb_dep]

### Create symlink for assets folder
copy_output = run_command(copy_cmd,meson.source_root()+'/assets',meson.current_build_dir()+'/assets', check: true)

##### main program #####
src = []
main_src = src + ['main.cpp']
main_exe = executable('main', main_src, dependencies: [dependencies], link_args:['-static-libgcc', '-static-libstdc++'], override_options:['cpp_std=c++17'])


##### testing part #####

### Get the necessary dependencvies for Google Test 
gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')

test_dependencies = dependencies + [gtest_dep, gmock_dep]

### List all test-files in a array and create a seperate Meson test for every test-file
test_src = src + ['image_test.cpp','init_vr_test.cpp','glfw_test.cpp','texture_test.cpp']
foreach single_test : test_src
    temp_exe = executable(single_test.strip('.cpp'),[single_test]+['test_main.cpp'], dependencies:test_dependencies, link_args:['-static-libgcc', '-static-libstdc++'], override_options:['cpp_std=c++17'])
    test(single_test.strip('_test.cpp'),temp_exe)
endforeach

### Create a Test which includes all other tests
test_src += 'test_main.cpp'
#test_exe = executable('all_tests',test_src, dependencies:test_dependencies, link_args:['-static-libgcc', '-static-libstdc++'], override_options:['cpp_std=c++17'])
#test('all_tests', test_exe)