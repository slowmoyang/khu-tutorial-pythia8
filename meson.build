project(
  'hh-pythia8',
  'cpp',
  default_options : [
    'cpp_std=c++23',
    'warning_level=2',
  ],
)

cpp = meson.get_compiler('cpp')

# pythia8
pythia8_config = find_program('pythia8-config', required: true)
# pythia8-config --cxxflags ships -std=c++11; meson appends dependency compile_args
# after its own -std= (cpp_std above), so an unfiltered flag silently downgrades the
# standard and breaks C++17+ headers (e.g. argparse's std::string_view). Strip it.
pythia8_compile_args = []
foreach arg : run_command(pythia8_config, '--cxxflags', check: true).stdout().strip().split(' ')
  if arg != '' and not arg.startswith('-std=')
    pythia8_compile_args += arg
  endif
endforeach
pythia8_link_args = run_command(pythia8_config, '--ldflags', '--libs', check: true).stdout().strip().split(' ')
# --includedir returns absolute paths; declare_dependency's include_directories
# keyword rejects absolute source-tree paths, so pass them as -I compile args.
pythia8_include_args = []
foreach dir : run_command(pythia8_config, '--includedir', check: true).stdout().strip().split(' ')
  if dir != ''
    pythia8_include_args += '-I' + dir
  endif
endforeach
pythia8_dep = declare_dependency(
    compile_args: pythia8_compile_args + pythia8_include_args,
    link_args: pythia8_link_args,
)

# hepmc3
hepmc3_config = find_program('HepMC3-config', required: true)
hepmc3_compile_args = run_command(hepmc3_config, '--cppflags', check: true).stdout().strip().split(' ')
hepmc3_link_args = run_command(hepmc3_config, '--ldflags', check: true).stdout().strip().split(' ')
hepmc3_include_args = []
foreach dir : run_command(hepmc3_config, '--includedir', check: true).stdout().strip().split(' ')
  if dir != ''
    hepmc3_include_args += '-I' + dir
  endif
endforeach
hepmc3_dep = declare_dependency(
    compile_args: hepmc3_compile_args + hepmc3_include_args,
    link_args: hepmc3_link_args,
)


argparse_dep = dependency(
  'argparse',
  required: true,
)


executable(
  'run-pythia8',
  'src/run-pythia8.cc',
  dependencies : [
    pythia8_dep,
    hepmc3_dep,
    argparse_dep,
  ],
  install : false,
)
