#ifndef PAV_EFFECT
#define PAV_EFFECT

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace upc {
  class Effect {
  public:
	Effect() {}
	virtual void command(unsigned int) = 0;
    virtual ~Effect() {};
	virtual void operator()(std::vector<float> &x) = 0;
  };

  Effect * get_effect(const std::string &name, const std::string &str_parameters = "");
}

#endif
