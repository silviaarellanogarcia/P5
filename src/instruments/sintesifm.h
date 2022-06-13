#ifndef SINTESIFM
#define SINTESIFM

#include <vector>
#include <string>
#include "instrument.h"
#include "envelope_adsr.h"

namespace upc {
  class SintesiFM: public upc::Instrument {
    EnvelopeADSR adsr;
    float fc, fm, step1, step2, phase1, phase2;
	  float A, I, N1, N2;
    std::vector<float> tbl;
  public:
    SintesiFM(const std::string &param = "");
    void command(long cmd, long note, long velocity=1); 
    const std::vector<float> & synthesize();
    bool is_active() const {return bActive;} 
  };
}

#endif