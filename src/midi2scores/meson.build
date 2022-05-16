inc = include_directories(['include',])

# Librería MIDIFILE

src = [
    'src/midi2skini.cpp',
    'src/FileIO.cpp',
    'src/MidiFile.cpp',
    'src/Options.cpp',
    'src/Options_private.cpp',
]

executable(
    'midi2skini',
    sources: src,
    include_directories: inc,
    link_args: ['-lm', '-lsndfile'],
    cpp_args: ['-DLINUX'],
    install: true,
)

midi2sco = 'midi2sco.py'
source = join_paths(meson.source_root(), 'src/midi2scores', midi2sco)   # Ruta completa
prog = midi2sco.split('.')[0]                                             # Nombre sin extensión
dest = join_paths(get_option('prefix'), get_option('bindir'), prog)
custom_target(midi2sco,
    input : source,
    output : prog,
    command : ['ln', '-sf', source, dest],
    build_by_default: true,
)
