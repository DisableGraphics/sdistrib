project('sdistrib-client', 'cpp', version: '0.0.1', default_options : ['cpp_std=c++20'])

sources = [
	'src/main.cpp',
	'src/parse.cpp'
]

zmqdep = dependency('cppzmq')
msgpackdep = dependency('msgpack-cxx')

inc_dir = include_directories('../common')

executable('sdistrib-client', sources, dependencies : [zmqdep, msgpackdep], include_directories : inc_dir)