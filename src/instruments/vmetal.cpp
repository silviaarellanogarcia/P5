#include <iostream>
#include <math.h>
#include "vmetal.h"
#include "keyvalue.h"

#include <stdlib.h>

using namespace upc;
using namespace std;

Vmetal::Vmetal(const std::string &param) 
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
  
  if (!kv.to_float("N1",N1))
    N1 = 5; //default value

  if (!kv.to_float("N2",N2))
    N2 = 6; //default value

  if (!kv.to_float("I",I))
    I = 1; //default value

  if (!kv.to_float("T",T))
    T = 0.2; //default value

  if (!kv.to_float("A",A))
    A = 1; //default value

  I = 1. - pow(2, -I / 12.); // Pasar I de semitonos a lineal
}


void Vmetal::command(long cmd, long note, long vel) {
  if (cmd == 9) {		//'Key' pressed: attack begins
    bActive = true;
    adsr.start();
    
    fc = (pow(2, ((note - 69.) / 12.)) * 440.) / SamplingRate; // Página 14, ecuación 3 del guión de prácticas
    fm = (N2/N1) * fc;
    step1 = 2 * M_PI * fm;
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

const vector<float> & Vmetal::synthesize() {
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
  

  for (unsigned int i = 0; i < x.size(); i++) {
    if(x[i] >= T){
      x[i] = T;
    }else if(x[i] < -T){
      x[i] = -T;
    }
    x[i] = A * x[i];
  }

  adsr(x); //apply envelope to x and update internal status of ADSR
  return x;
}
