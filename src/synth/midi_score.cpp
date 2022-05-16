#include <sstream>
#include <fstream>
#include "midi_score.h"

using namespace std;
using namespace upc;

namespace upc {
  MidiMsg::MidiMsg(const std::string &s) {
    istringstream istr(s);
    if (istr >> lTimeTick >> cEvent >> cChannel >> param[0] >> param[1])
      bOk = true;
    else
      bOk = false;
  }

  int read_midi_score(const string &filename, std::vector<MidiMsg> &v) {
    v.clear();
    ifstream is(filename.c_str());
    if (!is.good())
      return -1;

    string s;
    int nline = 0;
    while (getline (is, s)) {
      ++nline;
      string::size_type pos = s.find('#');
      if (pos != string::npos) s.erase(pos);
      if (s.empty()) continue;
	
      MidiMsg msg(s);
      if (!msg.ok()) {
        cerr << "Warning: format error; skiping line "
             << nline << " from file " << filename << endl;
        cerr << s << "\n----------------\n";
        continue;
      }
      v.push_back(msg);
    }
    if (v.empty())
      return -2;
    else
      return 0;
  }

  ostream & operator<<(ostream &os, const MidiMsg &msg) {
    if (!msg.ok())
      return os;

    os << msg.getTick() << '\t'
       << msg.getEvent() << '\t'
       << msg.getChannel() << '\t'
       << msg.getParam1() << '\t'
       << msg.getParam2();

    return os;
  }
}
