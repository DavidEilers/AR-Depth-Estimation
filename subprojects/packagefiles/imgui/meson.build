project('imgui', 'cpp',
    version: '1.81',
    license: 'MIT',
)

gl_dep = dependency('gl', required: true)

glfw_proj = subproject('glfw')
glfw_dep = glfw_proj.get_variable('glfw_dep')

sources = [
    'imgui_draw.cpp',
    'backends/imgui_impl_glfw.cpp',
    'backends/imgui_impl_opengl3.cpp',
    'imgui_widgets.cpp',
    'imgui.cpp'
]

imgui_lib = library('imgui',
                       sources,
                       version: meson.project_version(),
                       cpp_args: ['-DIMGUI_IMPL_OPENGL_LOADER_GLAD2'],
                       dependencies: glfw_dep,
                       include_directories: [include_directories('.'),
                                             include_directories('backends')])
imgui_dep = declare_dependency(link_with: imgui_lib,
                              include_directories:
                                    [include_directories('.'),
                                    include_directories('backends')])