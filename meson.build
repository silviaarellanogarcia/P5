project(
    'Práctica 5 de PAV - Síntesis musical polifónica', 'cpp',
    default_options: 'cpp_std=c++11',
    version: 'v2.0'
)

inc = include_directories(['src/include', 'src/synth', 'src/instruments', 'src/pav/docopt_cpp'])

# Librería PAV
subdir('src/pav')

# Programas del proyecto
subdir('src')

# Programa midi2skini
subdir('src/midi2scores')

# Documentación del proyecto
subdir('src/doxyfile')
