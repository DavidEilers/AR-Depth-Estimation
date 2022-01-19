##### main program #####
src = []
main_src = src + 'main.cpp'
main_exe = executable('main', main_src)


##### testing part #####
gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')

test_src = src + ['image_test.cpp']
test_src += 'test_main.cpp'
test_exe = executable('test_main',test_src, dependencies:gtest_dep)
test('basic', test_exe)