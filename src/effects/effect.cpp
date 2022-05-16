#include <iostream>
#include "tremolo.h"
#include "vibrato.h"

/*
  For each new effect:
  - Add the header in this file
  - Add the call to the constructor in get_effect() (also in this file)
  - Add the source file to src/meson.build
*/

using namespace std;

namespace upc {
  Effect *get_effect(const string &name, const string &parameters) {
    Effect *pEffect = 0;
    //    cout << name << ": " << parameters << endl;
    if (name == "Tremolo") {
      pEffect = (Effect *) new Tremolo(parameters);
    }
	else if (name == "Vibrato") {
      pEffect = (Effect *) new Vibrato(parameters);
    }
    return pEffect;
  }
}
