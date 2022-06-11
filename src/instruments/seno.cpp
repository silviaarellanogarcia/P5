#include <iostream>
#include <math.h>
#include "seno.h"
#include "keyvalue.h"

#include <stdlib.h>

using namespace upc;
using namespace std;

Seno::Seno(const std::string &param) 
  : adsr(SamplingRate, param) {
  bActive = false;
  x.resize(BSIZE);

  /*
    You can use the class keyvalue to parse "param" and configure your instrument.
    Take a Look at keyvalue.h    
  */
  KeyValue kv(param);
  int N;

  if (!kv.to_int("N",N))
    N = 40; //default value
  
  //Create a tbl with one period of a sinusoidal wave
  tbl.resize(N);
  float phase = 0, istep = 0, step = 2 * M_PI /(float) N;
  index = 0;
  for (int i=0; i < N ; ++i) {
    tbl[i] = sin(phase);
    phase += step;
  }
}


void Seno::command(long cmd, long note, long vel) {
  if (cmd == 9) {		//'Key' pressed: attack begins
    bActive = true;
    adsr.start();
    
    f0 = pow(2, ((note - 69.) / 12.)) * 440.; // Página 14, ecuación 3 del guión de prácticas
    istep = (f0 / SamplingRate) * tbl.size();
    index = 0;
      A = vel / 127.; // Amplitud
  }
  else if (cmd == 8) {	//'Key' released: sustain ends, release begins
    adsr.stop();
  }
  else if (cmd == 0) {	//Sound extinguished without waiting for release to end
    adsr.end();
  }
}


const vector<float> & Seno::synthesize() {
  if (not adsr.active()) {
    x.assign(x.size(), 0);
    bActive = false;
    return x;
  }
  else if (not bActive)
    return x;

  for (unsigned int i=0; i<x.size(); ++i) {
    // x[i] = A * tbl[index++]; // Sin interpolación. No se tienen en cuenta las muestras anteriores.

    // Los índices seguramente no sean valores enteros. Por lo tanto, hay que interpolar, teniendo  en cuenta la cercanía de las muestras de al lado.
    if ((unsigned int)ceil(index) == tbl.size()){ // El último elemento, es un caso especial porque salta a la primera muestra
      x[i] = A * (((1 - (index - floor(index))) * tbl[floor(index)]) + ((1 - (ceil(index) - index)) * tbl[0])); 
    }else{
      x[i] = A * (((1 - (index - floor(index))) * tbl[floor(index)]) + ((1 - (ceil(index) - index)) * tbl[ceil(index)]));
    }

    index = fmod(index + istep, tbl.size());
  }
  
  adsr(x); //apply envelope to x and update internal status of ADSR

  return x;
}
