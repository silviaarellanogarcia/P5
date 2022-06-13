#include <iostream>
#include <math.h>
#include "clarinete.h"
#include "keyvalue.h"

#include <stdlib.h>

using namespace upc;
using namespace std;

Clarinete::Clarinete(const std::string &param) 
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

  //Valores del paper
  fcarrier = 900;
  fmodulating = 500;
  N1 = 4;
  N2 = 8;
  cm = fcarrier/fmodulating;

  //Create a tbl with one period of a sinusoidal wave
  tbl.resize(N);
  float phase = 0, step = 2 * M_PI /(float) N;
  for (int i=0; i < N ; ++i) {
    tbl[i] = sin(phase);
    phase += step;
  }
}


void Clarinete::command(long cmd, long note, long vel) {
  if (cmd == 9) {		//'Key' pressed: attack begins
    bActive = true;
    adsr.start();
    
    fc = (pow(2, ((note - 69.) / 12.)) * 440.) / SamplingRate; // Página 14, ecuación 3 del guión de prácticas
    fm = cm * fc;
    step1 = 2 * M_PI * fm;
    ///////******** ES COPIA DE SINTESI.FM, FALTA CORREGIR A PARTIR DE AQUI **************
    step2 = 2 * M_PI * fc;
    phase1 = 0;
    phase2 = 0;
    A = vel / 127.; // Amplitud
  }
  else if (cmd == 8) {	//'Key' released: sustain ends, release begins
    adsr.stop();
  }
  else if (cmd == 0) {	//Sound extinguished without waiting for release to end
    adsr.end();
  }
}


const vector<float> & Clarinete::synthesize() {
  if (not adsr.active()) {
    x.assign(x.size(), 0);
    bActive = false;
    return x;
  }
  else if (not bActive)
    return x;

  for (unsigned int i=0; i<x.size(); ++i) {
    x[i] = A*sin(phase1 + I*sin(phase2));

    phase1 += step1;
    phase2 += step2;

    while (phase1 >= 2*M_PI) {
      phase1 -= 2*M_PI;
    }
    while (phase2 >= 2*M_PI) {
      phase2 -= 2*M_PI; 
    }
  }
  
  adsr(x); //apply envelope to x and update internal status of ADSR

  return x;
}
