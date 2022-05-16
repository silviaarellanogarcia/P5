#ifndef _MIDI_SCORE_H
#define _MIDI_SCORE_H

#include <vector>
#include <iostream>

namespace upc {
  class MidiMsg {
  private:
    unsigned long lTimeTick;
    unsigned int cEvent;
    unsigned int cChannel;
    unsigned int param[2];
    bool bOk;
  public:
    MidiMsg(const std::string &s);
    bool ok() const {return bOk;}
    unsigned long getTick() const {return lTimeTick;}
    unsigned int getEvent() const {return cEvent;}
    unsigned int getChannel() const {return cChannel;}
    unsigned int getParam1() const {return param[0];}
    unsigned int getParam2() const {return param[1];}
  };

  int read_midi_score(const std::string &filename, std::vector<MidiMsg> &vmsg);
  std::ostream & operator<<(std::ostream &os, const MidiMsg &);

}



#endif
