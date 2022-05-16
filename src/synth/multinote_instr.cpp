#include <iostream>
#include "multinote_instr.h"
#include "keyvalue.h"

using namespace upc;
using namespace std;


InstrumentMN::InstrumentMN (const std::string &s_name, const std::string & s_par) : name(s_name), parameters(s_par) {
  bActive = false;
}

bool InstrumentMN::ready() const {
  Instrument *pInst = 0;
  pInst = get_instrument(name, parameters);
  if (pInst != 0) {
    delete pInst;
    return true;
  } else 
    return false;
}

void InstrumentMN::command(long cmd, long note, long velocity) {
  if (cmd != 9 && cmd != 8)
    return;

  Instrument *pInst = 0;
  iterator i = note_instruments.find(note);
  if (i != note_instruments.end()) {
    pInst = i->second;
  }  else if (cmd != 8) { //do not create an instrument just to stop it!
    pInst = get_instrument(name, parameters);
    note_instruments[note] = pInst;
  }

  if (pInst != 0) {
    pInst->command(cmd, note, velocity);
    if (!pInst->is_active())
      note_instruments.erase(note);
  }

  if (note_instruments.empty())
    bActive = false;
  else
    bActive = true;
  
}


const std::vector<float> & InstrumentMN::synthesize() {

  x.assign(x.size(),0);

  if (!is_active())
    return x;

  iterator i;
  int n_active = 0;
  bActive = false;
  for (i = note_instruments.begin(); i != note_instruments.end();) {
    const vector<float> & xr = i->second->synthesize();
    if (xr.size() != x.size()) {
      cerr << "ERROR: xr x size:" << xr.size() << '\t' << x.size() << endl;
      return x;
    }
    n_active++;
    for (unsigned n = 0; n < xr.size(); ++n)
      x[n] += xr[n];
    
    if (i->second->is_active()) {
      bActive = true;
      ++i;
    } else {
      note_instruments.erase(i++);
    }
  }

  /*  
  //Normalize amplitudes
  if (n_active > 1) {
    for (unsigned n = 0; n < x.size(); ++n)
      x[n] = x[n]/ (float) n_active;
  }
  */
  return x;
}
