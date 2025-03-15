project('sdistrib-server', 'cpp', version: '0.0.1', default_options : ['cpp_std=c++17'])

sources = [
	'src/main.cpp'
]

zmqdep = dependency('cppzmq')
msgpackdep = dependency('msgpack-cxx')

executable('sdistrib-server', sources, dependencies : [zmqdep, msgpackdep])