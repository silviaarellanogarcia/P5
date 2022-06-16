#include <iostream>
#include <math.h>
#include "fictabla.h"
#include "keyvalue.h"
#include "wavfile_mono.h"

#include <stdlib.h>

using namespace upc;
using namespace std;

FicTabla::FicTabla(const std::string &param) 
  : adsr(SamplingRate, param) {
  bActive = false;
  x.resize(BSIZE);

  /*
    You can use the class keyvalue to parse "param" and configure your instrument.
    Take a Look at keyvalue.h    
  */
  KeyValue kv(param);
  std::string file_name;
  static string kv_null;
  if ((file_name = kv("file")) == kv_null) {
    cerr << "Error: no se ha encontrado el campo con el fichero de la señal para un instrumento FicTabla" << endl;
    throw -1;
  }

  unsigned int fm;
  if (readwav_mono(file_name, fm, tbl) < 0) {
    cerr << "Error: no se puede leer el fichero " << file_name << " para un instrumento FicTabla" << endl;
    throw -1;
  }
}


void FicTabla::command(long cmd, long note, long vel) {
  if (cmd == 9) {		//'Key' pressed: attack begins
    bActive = true;
    adsr.start();
    
    f0 = pow(2, ((note - 69.) / 12.)) * 440.; // Página 14, ecuación 3 del guión de prácticas
    istep = tbl.size() * (f0 / SamplingRate);
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


const vector<float> & FicTabla::synthesize() {
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
    if (index == (int)index) {
      x[i] = A*tbl[index];
    } else {
      int upper = (int)ceil(index);
      int lower = (int)floor(index);
      x[i] = A*((upper - index)*tbl[lower % tbl.size()] + (index - lower)*tbl[upper % tbl.size()]);

      //cout << index << '\t' << upper << '\t' << lower << '\t' << A*tbl[upper % tbl.size()] << '\t' << A*tbl[lower % tbl.size()] << '\t' << x[i] << endl;
    }
    index += istep;
    if (index >= tbl.size()) {
      index -= tbl.size();
    }
  }
  
  adsr(x); //apply envelope to x and update internal status of ADSR

  return x;
}
