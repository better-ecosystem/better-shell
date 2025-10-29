project('Better Shell', 'cpp',
        default_options: [ 'cpp_std=c++20', 'warning_level=3' ],
        version: '0.0.1',
        license: 'GPL-3.0-or-later',
        license_files: [ 'COPYING' ])

add_project_dependencies([
    dependency('jsoncpp'),
    dependency('giomm-2.68'),
    dependency('glibmm-2.68'),
], language: 'cpp')

args = [
    '-DAPP_VERSION="@0@"'.format(meson.project_version()),
    '-DAPP_NAME="@0@"'.format(meson.project_name()),
    '-DAPP_ID="org.BetterDE.Better-Shell"',
]

subdir('src')
executable('better-shell', source_files,
           include_directories: include_directories('include',
                                                    'extern/utf8cpp',
                                                    'extern/utf8cpp/utf8'),
           install: true,
           cpp_args: args)