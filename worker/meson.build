project('sdistrib-worker', 'cpp', version: '0.0.1', default_options : ['cpp_std=c++20'])

sources = [
	'src/main.cpp',
]

# Get stable-diffusion.cpp as a dependency
cmake = import('cmake')

opt_var = cmake.subproject_options()
opt_var.add_cmake_defines({get_option('platform'): true})
opt_var.add_cmake_defines({'SD_FAST_SOFTMAX': true})
sub_proj = cmake.subproject('stable-diffusion.cpp', options: opt_var)

sdcpp = sub_proj.dependency('stable-diffusion')
ggml = [sub_proj.dependency('ggml'), sub_proj.dependency('ggml-cpu'), sub_proj.dependency('ggml-cuda'), sub_proj.dependency('ggml-base'), sub_proj.dependency('zip')]

zmqdep = dependency('cppzmq')
msgpackdep = dependency('msgpack-cxx')

inc_dir = include_directories('../common')

executable('sdistrib-worker', sources, dependencies : [ggml, zmqdep, msgpackdep, sdcpp], include_directories : inc_dir)