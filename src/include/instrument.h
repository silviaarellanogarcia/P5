#ifndef PAV_INSTRUMENT
#define PAV_INSTRUMENT

#include <vector>
#include <map>
#include <list>
#include <string>
#include "effect.h"

const int BSIZE = 32;
const long SamplingRate = 44100;

namespace upc {

  class Instrument {
  protected:
    std::vector<float> x; //samples
    bool bActive;
  public:
    Instrument() : x(BSIZE) {}
    virtual void command(long cmd, long note, long velocity=100) = 0; 
    virtual const std::vector<float> & synthesize() = 0;
    bool is_active() const {return bActive;} 
    virtual ~Instrument() {};

    typedef std::map<unsigned int, Effect *> Effects;
    Effects effects;
  };

  Instrument * get_instrument(const std::string &name, 
			      const std::string &str_parameters = "");

}


#endif

