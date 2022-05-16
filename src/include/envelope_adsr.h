#ifndef ENVELOPE_ADSR
#define ENVELOPE_ADSR

#include <vector>
#include <string>

namespace upc {

  class EnvelopeADSR {
    unsigned int rate;
    unsigned int n_pressed, n_released;
    bool bAttack, bRelease;
    std::vector<float> attack, release;
    float a_sustain, factor_slope; 
    float t_attack, t_decay, t_release;

  public:
    EnvelopeADSR(unsigned int samplingRate,
                 float Attack=0.1,
                 float Decay=0.05,
                 float Sustain = 0.5F,
                 float Release=0.05,
                 float factor_slope = 1.5F);

    EnvelopeADSR(unsigned int samplingRate,
		 const std::string &strParam);

    int set(float Attack=0.1,
	    float Decay=0.05,
	    float Sustain = 0.5F,
	    float Release=0.05,
	    float factor_slope = 1.5F);

    int set (const std::string &strParam);

    void start();
    void stop();
    void end();
    bool active();
    bool operator()(std::vector<float> & envelope);
  };
}

#endif
