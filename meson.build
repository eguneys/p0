project('p0', 'cpp',
              default_options : ['cpp_std=c++14'],
              meson_version: '>=0.45')

cc = meson.get_compiler('cpp')

if cc.get_id() == 'clang'
  # Thread safety annotation
  add_project_arguments('-Wthread-safety', language : 'cpp')
endif
if cc.get_id() == 'clang' or cc.get_id() == 'gcc'
  add_project_arguments('-Wextra', language : 'cpp')
  add_project_arguments('-pedantic', language : 'cpp')

  if get_option('buildtype') == 'release'
    add_project_arguments('-march=native', language : 'cpp')
  endif
endif


deps = []
files = []
includes = []
has_backends = false

includes += include_directories('third_party', is_system: true)

files += [
  'src/engine.cc',
  'src/version.cc',
  'src/soko/bitboard.cc',
  'src/soko/board.cc',
  'src/soko/position.cc',
  'src/soko/uciloop.cc',
  'src/mcts/params.cc',
  'src/mcts/node.cc',
  'src/mcts/search.cc',
  'src/neural/factory.cc',
  'src/neural/encoder.cc',
  'src/neural/network_random.cc',
  'src/selfplay/game.cc',
  'src/selfplay/loop.cc',
  'src/selfplay/tournament.cc',
  'src/utils/commandline.cc',
  'src/utils/configfile.cc',
  'src/utils/logging.cc',
  'src/utils/optionsdict.cc',
  'src/utils/optionsparser.cc',
  'src/utils/string.cc'
]

includes += include_directories('src')

files += 'src/utils/filesystem.posix.cc'
deps += [
     cc.find_library('pthread'),
     ]

executable('p0', 'src/main.cc',
                 files, include_directories: includes, dependencies: deps, install: true)

if get_option('gtest')
   gtest = dependency('gtest', fallback: ['gtest', 'gtest_dep'])
   p0_lib = library('p0_lib', files, include_directories: includes, dependencies: deps)

   test('SokoBoard',
      executable('sokoboard_test', 'src/soko/board_test.cc',
      include_directories: includes, link_with: p0_lib, dependencies: gtest), args: '--gtest_output=xml:sokoboard.xml', timeout: 90)

   test('EncodePositionForNN',
      executable('encoder_test', 'src/neural/encoder_test.cc',
      include_directories: includes, link_with: p0_lib, dependencies: gtest), args: '--gtest_output=xml:encoder.xml', timeout: 90)

endif