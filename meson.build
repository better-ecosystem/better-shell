project('Better Shell', 'cpp',
        default_options: [ 'cpp_std=c++20', 'warning_level=3' ],
        meson_version: '>=1.8.0',
        version: '0.0.1',
        license: 'GPL-3.0-or-later',
        license_files: [ 'COPYING' ])

utf8cpp = declare_dependency(
    include_directories: include_directories('/usr/include/utf8cpp')
)

add_project_dependencies([
    dependency('jsoncpp'),
    dependency('giomm-2.68'),
    dependency('glibmm-2.68'),
    utf8cpp
], language: 'cpp')

add_project_arguments([
    '-DAPP_VERSION="' + meson.project_version() + '"',
    '-DAPP_NAME="'    + meson.project_name()    + '"',
    '-DAPP_ID="org.BetterDE.Better-Shell"',
], language: 'cpp')

subdir('src/input')
subdir('src/parser')

files = files(
    'src/main.cc',
    'src/utils.cc',
) + input_files + parser_files

executable('better-shell', files,
           include_directories: include_directories('include'),
           install: true)