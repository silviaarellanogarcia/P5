#!/usr/bin/env  python3

import re
import os
from subprocess import run, PIPE
from subprocess import CalledProcessError as MidiFileError

def midi2sco(ficMIDI, ficScore, beats_per_minute=120, ticks_per_beat=144):
    try:
        midi = run(['midi2skini', ficMIDI], stdout=PIPE, check=True)
        mensajes = midi.stdout.decode('utf-8').split('\n')
    except MidiFileError as e:
        print(f"Error al abrir el fichero {ficMIDI} ({os.strerror(e.returncode)})")
        exit(1)

    reTempo = re.compile('tempo:\s*(?P<tempo>\d+)')
    for mensaje in mensajes:
        if reTempo.search(mensaje):
            tempo = reTempo.search(mensaje)['tempo']

            if beats_per_minute < 0:
                beats_per_minute = int(tempo)
            else:
                if beats_per_minute != int(tempo):
                    print(f'ATENCIÓN: encontrado tempo={tempo}, pero usando bpm={beats_per_minute}\n')
            break
    if beats_per_minute < 0:
        beats_per_minute = 120
        print(f'ATENCIÓN: no se ha encontrado el tempo. Usando el valor por defecto bpm={beats_per_minute}\n')

    print(f"Usando bpm={tempo} pulsos por minuto y tpb={ticks_per_beat:.0f} ticks por pulso. Se recomienda")
    print(f"usar estos mismos valores al reproducir {ficScore} con el programa 'synth'.\n")


    sOrden = '\s*(?P<Orden>NoteOn|NoteOff)'
    sTiempo = '\s+=(?P<Tiempo>[\d.]+)'
    sCanal = '\s+(?P<Canal>\d+)'
    sPitch = '\s+(?P<Pitch>\d+)'
    sVelocidad = '\s+(?P<Velocidad>\d+)'
    reNota = re.compile(sOrden + sTiempo + sCanal + sPitch + sVelocidad)

    with open(ficScore, 'wt') as fpScore:
        instruments = set()
        last_ticks = 0
        for mensaje in mensajes:
            comando = reNota.match(mensaje)
            if not comando:
                fpScore.write(f'# {mensaje}\n')
                continue

            orden = comando['Orden']
            tiempo = float(comando['Tiempo'])
            canal = int(comando['Canal'])
            pitch = int(comando['Pitch'])
            velocidad = int(comando['Velocidad'])

            instruments.add(canal)

            orden = 9 if orden == 'NoteOn' else 8
            ticks = int(0.5 + tiempo * beats_per_minute / 60 * ticks_per_beat)
            delta = ticks - last_ticks
            last_ticks = ticks

            fpScore.write(f"{delta}\t{orden}\t{canal}\t{pitch}\t{velocidad}\n")
        
        print(f'El fichero {ficMIDI} incluye {len(instruments)} instrumento(s):')
        for instrument in sorted(instruments):
            print('\t', instrument)
        print(f'Consulte el fichero {ficScore} para ver si proporciona información de los mismos.')


########################################################################################################
# Main Program
########################################################################################################

from docopt import docopt

USAGE='''
Converts a MIDI file into an ASCII "score" file format inspired in skini

Usage:
    midi2sco [--help|-h] [options] <file-midi> <file-score>

Options:
    --bpm FLOAT, -b FLOAT  Beats per minute: tempo [default: auto]
    --tpb FLOAT, -t FLOAT  Ticks per beat: must match input score  [default: 120]
    --help, -h             Shows this message

Arguments:
    <file-midi>   File with the input MIDI score
    <file-score>  File with the output ASCII score
'''

if __name__ == '__main__':
    args = docopt(USAGE)

    ficMIDI = args['<file-midi>']
    ficScore = args['<file-score>']
    bpm = args['--bpm']
    bpm = float(bpm) if bpm != 'auto' else -1
    tpb = float(args['--tpb'])

    midi2sco(ficMIDI, ficScore, bpm, tpb)
