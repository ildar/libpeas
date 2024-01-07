lua53_loader_name = 'lua53loader'

lua53_lgi_version = lua53_lgi_dep.version()
lua53_lgi_version_arr = lua53_lgi_version.split('.')
lua53_lgi_version_major = lua53_lgi_version_arr[0].to_int()
lua53_lgi_version_minor = lua53_lgi_version_arr[1].to_int()
lua53_lgi_version_micro = lua53_lgi_version_arr[2].to_int()

lua53_loader_c = files(
  'peas-lua-internal.c',
  'peas-lua-utils.c',
  'peas-plugin-loader-lua.c',
)

subdir('resources') # Declares 'lua53_loader_res'

lua53_loader_deps = [
  libpeas_dep,
  lua53_dep,
  lua53_lgi_dep,
  luajit_dep,
]

lua53_loader_c_args = [
  '-DHAVE_CONFIG_H',
  '-DLGI_MAJOR_VERSION=@0@'.format(lua53_lgi_version_major),
  '-DLGI_MINOR_VERSION=@0@'.format(lua53_lgi_version_minor),
  '-DLGI_MICRO_VERSION=@0@'.format(lua53_lgi_version_micro),
]

lua53_loader_sha = shared_module(
  lua53_loader_name,
  lua53_loader_c + lua53_loader_res,
  include_directories: rootdir,
  dependencies: lua53_loader_deps,
  c_args: project_c_args + lua53_loader_c_args,
  install: true,
  install_dir: join_paths(
    pkglibdir,
    'loaders',
  ),
  name_suffix: module_suffix
)
