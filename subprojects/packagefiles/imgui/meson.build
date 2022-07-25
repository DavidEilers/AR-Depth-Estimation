project('imgui', 'cpp',
    version: '1.81',
    license: 'MIT',
)

gl_dep = dependency('gl', required: true)

glfw_proj = subproject('glfw')
glfw_dep = glfw_proj.get_variable('glfw_dep')

glad_dep = glfw_proj.get_variable('glad_dep')

sources = [
    'imgui_demo.cpp',
    'imgui_tables.cpp',
    'imgui_draw.cpp',
    'backends/imgui_impl_glfw.cpp',
    'backends/imgui_impl_opengl3.cpp',
    'imgui_widgets.cpp',
    'imgui.cpp',
    'glad_gl.c'
]

#imgui_lib = library('imgui',
#                       sources,
#                       version: meson.project_version(),
#                       cpp_args: ['-DIMGUI_IMPL_OPENGL_LOADER_GLAD2'],
#                       dependencies: [glfw_dep,dependency('gl',required:true)],
#                       #link_with: [],
#                       include_directories: [include_directories('.'),
#                                             include_directories('backends')])
imgui_dep = declare_dependency(dependencies: [glfw_dep,dependency('gl',required:true)],
                                include_directories:
                                    [include_directories('.'),
                                    include_directories('backends')],
                                sources: sources)