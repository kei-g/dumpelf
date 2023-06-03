project('dumpelf', ['cpp'],
        default_options: ['cpp_std=c++20',
                          'warning_level=3',
                          'werror=true'])

add_project_arguments(['-fno-exceptions',
                       '-fno-rtti',
                       '-march=native'],
                      language: 'cpp')

add_project_link_arguments(['-Wl,--build-id=fast',
                            '-Wl,--discard-all',
                            '-Wl,--no-eh-frame-hdr',
                            '-Wl,--no-gnu-unique'],
                           language: 'cpp')

inc = include_directories(['inc'], is_system: false)
executable(meson.project_name(),
           cpp_pch: ['inc/stdafx.hh'],
           include_directories: [inc],
           sources: ['src/dump.cc',
                     'src/elf.cc',
                     'src/formatter.cc',
                     'src/ini.cc',
                     'src/load.cc',
                     'src/main.cc',
                     'src/stdafx.cc'])
