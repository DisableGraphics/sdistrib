project('sdistrib-worker', 'cpp', version: '0.0.1', default_options : ['cpp_std=c++20'])

sources = [
	'src/main.cpp',
]

zmqdep = dependency('cppzmq')
msgpackdep = dependency('msgpack-cxx')

inc_dir = include_directories('../common')

executable('sdistrib-worker', sources, dependencies : [zmqdep, msgpackdep], include_directories : inc_dir)