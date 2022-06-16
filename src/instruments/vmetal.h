#ifndef VMETAL
#define VMETAL

#include <vector>
#include <string>
#include "instrument.h"
#include "envelope_adsr.h"

namespace upc {
  class Vmetal: public upc::Instrument {
    EnvelopeADSR adsr;
    float fc, fm, step1, step2, phase1, phase2;
	  float A, I, N1, N2, T;
    std::vector<float> tbl;
  public:
    Vmetal(const std::string &param = "");
    void command(long cmd, long note, long velocity=1); 
    const std::vector<float> & synthesize();
    bool is_active() const {return bActive;} 
  };
}

#endif
