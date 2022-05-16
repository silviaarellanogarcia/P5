#ifndef TREMOLO_H
#define TREMOLO_H

#include <vector>
#include <string>
#include "effect.h"

namespace upc {
  class Tremolo: public upc::Effect {
    private:
      float fase, inc_fase;
	  float	fm, A;
    public:
      Tremolo(const std::string &param = "");
	  void operator()(std::vector<float> &x);
	  void command(unsigned int);
  };
}

#endif
