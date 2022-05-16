#include <iostream>
#include <math.h>
#include "vibrato.h"
#include "keyvalue.h"
#include "instrument.h"

#include <stdlib.h>

using namespace upc;
using namespace std;

Vibrato::Vibrato(const std::string &param) {
  KeyValue kv(param);

  // I is the maximum downward pitch shift in semitones
  // As the shift is sinusoidal, the maximum posible shift is 2 (one octave)
  // In this way, we prevent negative shifts
  if (!kv.to_float("I", I))
    I = 1; //default downward variation shift in semitones
  // Pass I in semitones to linear I
  I = 1. - pow(2, -I / 12.);

  if (!kv.to_float("fm", fm))
    fm = 10; //default value

  fase_sen = 0;
  fase_mod = 0;

  inc_fase_mod = 2 * M_PI * fm / SamplingRate;
}

void Vibrato::command(unsigned int cmd) {
  fase_mod = 0;
  if (cmd == 0) buffer.resize(0);
}

void Vibrato::operator()(std::vector<float> &x){
	std::vector<float> xout(x.size());
	unsigned int tot = 0;
	float	xant, xpos, rho;

	// Usamos resta en la modulación para garantizar que no nos quedaremos sin
	// muestras en el vector x: fase_sen += 1 - I * sin(fase_mod).
	//
	// En el fondo, esta trampa permite garantizar que el sistema es siempre causal...

	// Si el buffer no está vacío tomamos los valores de él hasta vaciarlo (o no).
	for (tot = 0; fase_sen < buffer.size() and tot < x.size(); tot++) {
		xant = buffer[(int) fase_sen];
		xpos = ((unsigned int) fase_sen < buffer.size() - 1 ? buffer[(int) fase_sen + 1] : x[0]);
		rho = fase_sen - (int) fase_sen;

		xout[tot] = xant + rho * (xpos - xant);

		fase_sen += 1 - I * sin(fase_mod);
		fase_mod += inc_fase_mod;
	}
	if (fase_sen > buffer.size()) {
		fase_sen -= buffer.size();
		buffer.resize(0);
	}
	else {
		buffer.erase(buffer.begin(), buffer.begin() + (int) fase_sen);
		fase_sen -= (int) fase_sen;
	}

	// Completamos la señal con muestras del vector actual
	while (tot < x.size()) {
		// Si podemos, interpolamos; si no, extrapolamos
		if (fase_sen < x.size() - 1) {
			xant = x[(int) fase_sen];
			xpos = x[(int) fase_sen + 1];
			rho = fase_sen - (int) fase_sen;
		}
		else {
			xant = x[(int) fase_sen - 1];
			xpos = x[(int) fase_sen];
			rho = fase_sen - (int) fase_sen + 1;
		}
		xout[tot] = xant + rho * (xpos - xant);

		if (++tot < x.size()) {
			fase_sen += 1 - I * sin(fase_mod);
			fase_mod += inc_fase_mod;
		}
	}

	// Guardamos los valores restantes de x en el buffer
	buffer.insert(buffer.end(), x.begin() + (int) fase_sen, x.end());
	fase_sen -= (int) fase_sen;

	while (fase_mod > M_PI) fase_mod -= 2 * M_PI;

	// Copimos los valores de xout en x
	x = xout;
}

