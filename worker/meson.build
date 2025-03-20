project('sdistrib-worker', 'cpp', version: '0.0.1', default_options : ['cpp_std=c++20'])

sources = [
	'src/main.cpp',
]

# Get stable-diffusion.cpp as a dependency
cmake = import('cmake')

opt_var = cmake.subproject_options()
opt_var.add_cmake_defines({'GGML_CCACHE': false})
opt_var.add_cmake_defines({get_option('platform'): true})
sub_proj = cmake.subproject('stable-diffusion.cpp', options: opt_var)
sdcpp = sub_proj.dependency('stable-diffusion')
ggml = sub_proj.dependency('ggml')

zmqdep = dependency('cppzmq')
msgpackdep = dependency('msgpack-cxx')

inc_dir = include_directories('../common')

executable('sdistrib-worker', sources, dependencies : [ggml, zmqdep, msgpackdep, sdcpp], include_directories : inc_dir)