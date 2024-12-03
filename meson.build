project('chihiro', 'c', default_options : ['c_std=c99'])

concord_deps = [dependency('concord'), dependency('threads'), dependency('libcurl')]
sqlite = dependency('sqlite3')

src = ['src/main.c', 'src/commands/kick.c', 'src/commands/ban.c', 'src/util/file.c']

executable('chihiro', sources : src, dependencies : [concord_deps, sqlite], link_args : '-ldotenv-s')
