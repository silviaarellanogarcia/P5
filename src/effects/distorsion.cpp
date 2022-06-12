#include <iostream>
#include <math.h>
#include "distorsion.h"
#include "keyvalue.h"

#include <stdlib.h>

using namespace upc;
using namespace std;

static float SamplingRate = 44100;

Distorsion::Distorsion(const std::string &param) {
  // La distorsión consiste en recortar la señal que supere un cierto umbral para provocar un efecto de que la señal está "rota".
  fase = 0;

  KeyValue kv(param);

  if (!kv.to_float("A", A))
    A = 1; // Valor de amplificación

  if (!kv.to_float("T", T))
    T = 0.7; // Umbral a partir del que se recorta la señal.
}

void Distorsion::command(unsigned int comm) {
  if (comm == 1) fase = 0;
}

void Distorsion::operator()(std::vector<float> &x){
  for (unsigned int i = 0; i < x.size(); i++) {
    if(x[i] >= T){
      x[i] = T;
    }else if(x[i] < -T){
      x[i] = -T;
    }

    x[i] = A * x[i];
  }
}
