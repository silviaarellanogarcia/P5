#include <iostream>
#include <fstream>
#include <sstream>
#include "orchest.h"
#include "multinote_instr.h"
#include "effect.h"

using namespace std;

namespace upc {

  Orchest::Orchest(const std::string conf_filename, float gain) : xt(BSIZE), anyActive(false), bReady(false) {
	this->gain = gain;
    bReady = false;
    ifstream is(conf_filename.c_str());

    //Read instrument configuration file
    if (!is.good())
      return;

    string s;
    int nline = 0;

    while (getline (is, s)) {
      ++nline;

      //Delete comments, from #
      string::size_type pos = s.find('#');
      if (pos != string::npos)
	      s.erase(pos);

      if (s.size() == 0) //skip empty lines
	      continue;

      istringstream istr(s); //convert string is stringstream, to use '>>'
      unsigned int channel;
      string name, param;
      if (istr >> channel >> name) {
	      string param;
	      getline(istr, param);
        if (register_instrument(channel, name, param)) {
	        cerr << "Error registering instrument: " << name << '\t' << param << endl;
	      }
      } else {
	      cerr << "Error parsing configuration file. LINE " << nline << ": " << s << endl;
	      return;
      }
    }
    bReady = true;

  }

  int Orchest::register_instrument(unsigned int channel, const string &name, const string &parameters) {
    InstrumentMN * pInstMN = new InstrumentMN(name, parameters); //get_instrument(name, parameters);
    if (pInstMN == 0 || !pInstMN->ready()) //Error creating instrument!
      return -1;
    else {
      Instrument *pInst = (Instrument *) pInstMN;
      cout << "Register instrument: " << channel << ' ' << name << endl;
      iterator i = instruments.find(channel);
      if (i != instruments.end()) {
	      delete i->second;
	      i->second = pInst;
      } else {
	      instruments[channel] = pInst;
      }
      return 0;
    }
  }

  int Orchest::register_effects(const std::string effects_file) {
    ifstream es(effects_file.c_str());

    //Read instrument configuration file
    if (!es.good())
      return -1;

    string s;
    int nline = 0;

    while (getline (es, s)) {
      ++nline;

      //Delete comments, from #
      string::size_type pos = s.find('#');
      if (pos != string::npos)
	      s.erase(pos);

      if (s.size() == 0) //skip empty lines
	      continue;

      istringstream istr(s); //convert string is stringstream, to use '>>'
      unsigned int effect;
      string name, param;
      if (istr >> effect >> name) {
	    getline(istr, param);
    	Effect *pEffect = get_effect(name, param);
        if (!pEffect) {
	      cerr << "Error registering effect: " << name << '\t' << param << endl;
	      return -1;
	    }

        cout << "Register effect: " << effect << ' ' << name << endl;

	    delete map_effects[effect];
	    map_effects[effect] = pEffect;
      }
	  else {
	      cerr << "Error parsing configuration file. LINE " << nline << ": " << s << endl;
	      return -1;
      }
    }
    return 0;
  }

  int Orchest::release_instrument(unsigned int channel) {
    iterator i = instruments.find(channel);
    if (i != instruments.end()) {
      delete i->second;
      instruments.erase(i);
      return 0;
    } else
      return -1;  //Not found (?)
  }

  int Orchest::command(unsigned int channel, unsigned int cmd,
			   unsigned int param1,  unsigned int param2) {

    iterator i = instruments.find(channel);
	if (i == instruments.end()) {
      //      cerr << "No instrument registered in channel " << channel << '\n';
	  return -1;
	}

    if (cmd == 0 || cmd == 8 || cmd == 9) {
      i->second->command(cmd, param1, param2);
      if (i->second->is_active())
	      anyActive = true;
      return 0;
    }
	else if (cmd == 12) {
	    i->second->effects.erase(param1);
		if (param2) {
		  i->second->effects[param1] = map_effects[param1];
		  if (not i->second->effects[param1]) {
		  	  i->second->effects.erase(param1);
      		  // cerr << "No effect registered with number " << param1 << '\n';
			  return -1;
		  }
		  i->second->effects[param1]->command(param2);
		}
	}
	else {
	  return -1;
	}

	return 0;
  }

  const vector<float> & Orchest::synthesize(){
    xt.assign(xt.size(),0);
    if (!anyActive)
      return xt;

    iterator i;
    int num_inst = 0;
    for (i = instruments.begin(); i != instruments.end(); ++i) {
      if (i->second->is_active()) {
	    vector<float> & xr = (vector<float> &) i->second->synthesize();
	    if (xr.size() != xt.size()) {
	      cerr << "ERROR: xr xt size:" << xr.size() << '\t' << xt.size() << endl;
	      return xt;
	    }
		for (Instrument::Effects::iterator e = i->second->effects.begin(); e != i->second->effects.end(); e++) {
			(*e->second)(xr);
		}
	    for (unsigned n = 0; n < xr.size(); ++n)
	      xt[n] += xr[n];
	    num_inst++;
	    if (i->second->is_active())
	      anyActive = true;
      }
    }

    for (unsigned n = 0; n < xt.size(); ++n)
      xt[n] *= gain;

    return xt;
  }
}
