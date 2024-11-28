project('chihiro', 'c', default_options : ['c_std=c99'])

concord_deps = [dependency('concord'), dependency('threads'), dependency('libcurl')]

src = ['src/main.c', 'src/commands/kick.c']

executable('chihiro', sources : src, dependencies : [concord_deps], link_args : '-ldotenv-s')
