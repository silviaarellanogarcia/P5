#include <iostream>
#include "envelope_adsr.h"
#include "keyvalue.h"

using namespace std;

namespace upc {
  EnvelopeADSR::EnvelopeADSR(unsigned int samplingRate, float A,
                             float D, float S,
                             float R, float fslope) : 
    rate(samplingRate)  {
    set(A, D, S, R, fslope);
  }

  EnvelopeADSR::EnvelopeADSR(unsigned int samplingRate, const std::string &strParam) : 
    rate(samplingRate) {
    set(); //set default values 
    set(strParam);
  }


  int EnvelopeADSR::set(const std::string &strParam) {
    KeyValue kv(strParam);

    float f;
    if (kv.to_float("ADSR_A", f))  t_attack = f;
    if (kv.to_float("ADSR_D", f))  t_decay = f;
    if (kv.to_float("ADSR_S", f))  a_sustain = f;
    if (kv.to_float("ADSR_R", f))  t_release = f;
    if (kv.to_float("ADSR_FS", f)) factor_slope = f;

    set(a_sustain, t_attack, t_decay, t_release, factor_slope);
    return 0;
  }


  int EnvelopeADSR::set(float S, float A, float D, float R, float fslope) {

    a_sustain = S;
    t_attack = A; 
    t_decay = D;
    t_release = R;
    factor_slope = fslope;

    bAttack = bRelease = false;
    
    unsigned int n_attack  = (int) (0.5 + t_attack * rate);
    unsigned int n_decay   = (int) (0.5 + t_decay * rate);

    n_pressed = n_attack + n_decay;
    n_released = (int) (0.5 + t_release * rate);

    if (n_pressed > 0)
      attack.resize(n_pressed);

    unsigned int n;
    float value = 0, step = 1.0/(n_attack-1);
    for (n=0; n < n_attack; ++n) {
      attack[n] = value;
      value += step;
    }
    step = (1.0-a_sustain)/(n_decay-1);
    for (; n < n_pressed; ++n) {
      attack[n] = value;
      value -= step;
    }
    release.resize(n_released);
    value = 1.0F;
    step = 1.0/(n_released-1);
    for (n=0; n < n_released; ++n) {
      release[n] = value;
      value -= step;
    }
    return 0;
  }

  void EnvelopeADSR::start() {
    bAttack = true;
    bRelease = false;
    n_pressed = 0;
  }

  void EnvelopeADSR::stop() {
    if (!bAttack) //Cannot release if not pressed
      return;

    bAttack = false;
    bRelease = true;
    n_released = 0;
  }

  void EnvelopeADSR::end() {
    bAttack = false;
    bRelease = false;
  }

  bool EnvelopeADSR::active() {
    return bAttack || bRelease;
  }

  using namespace std;
  bool EnvelopeADSR::operator()(vector<float> & x) {    
    unsigned int i;
    if (bAttack) {
      for (i = 0; i < x.size(); i++) {
        x[i] *= attack[n_pressed];
        if (n_pressed < attack.size()-1)
          n_pressed++;
      }
      return true;
    } else if (bRelease) {
      float f = 1.0F;
      if (n_pressed > 0) 
	f = attack[n_pressed-1];
      for (i = 0; i < x.size() && n_released < release.size(); i++)
        x[i] *=  f * release[n_released++];
      if (n_released == release.size()) {
        bRelease = false;
        for (; i < x.size(); i++)
          x[i] = 0;
      }
      return true;
    } else {
      x.assign(x.size(), 0);
      return false;
    }
  }
}
