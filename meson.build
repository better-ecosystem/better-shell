project('Better Shell', 'cpp',
        default_options: [ 'cpp_std=c++20', 'warning_level=3' ],
        meson_version: '>=1.8.0',
        version: '0.0.1',
        license: 'GPL-3.0-or-later',
        license_files: [ 'COPYING' ])

add_project_dependencies([
    dependency('jsoncpp'),
    dependency('giomm-2.68'),
    dependency('glibmm-2.68'),
], language: 'cpp')

add_project_arguments([
    '-DAPP_VERSION="' + meson.project_version() + '"',
    '-DAPP_NAME="'    + meson.project_name()    + '"',
    '-DAPP_ID="org.BetterDE.Better-Shell"',
], language: 'cpp')

subdir('src')

executable('better-shell', source_files,
           include_directories: include_directories('include',
                                                    'extern/utf8cpp',
                                                    'extern/utf8cpp/utf8'),
           install: true)