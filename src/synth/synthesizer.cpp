#include "orchest.h"
#include "sndfile.h"
#include "midi_score.h"
#include "docopt.h"
#include <set>
using namespace std;
using namespace upc;

const int PRINT_SCORE = 1;

void warn_unregistered_instruments(const vector<MidiMsg> &score, 
				   const Orchest &orchest, ostream &os);
void warn_unregistered_effects(const vector<MidiMsg> &score, const Orchest &orchest, ostream &os);

static const char USAGE[] = R"(
synth - Polyphonic Musical Synthesizer

Usage:
    synth [options] <instruments-file> <score-file> <output-wav>
    synth (-h | --help)
    synth --version

Options:
    -b FLOAT, --bpm=FLOAT      Beats per minute: tempo [default: 120]
    -t FLOAT, --tpb=FLOAT      Ticks per beat: must match input score  [default: 120]
    -g FLOAT, --gain=FLOAT     Gain applied to the output waveform [default: 0.5]
    -e STR, --effect-file=STR  Text file with the definition of the effects
    -v, --verbose              Show the score on screen as it is played
    -h, --help                 Show this screen
    --version                  Show the version of the project

Arguments:
    instruments-file           Text file with the definition of the instruments
    score-file                 Text file with the score to be played
    output-wav                 Wave file with synthesized score
)";

int main(int argc, const char **argv) {  
  std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
      {argv + 1, argv + argc},	// array of arguments, without the program name
      true,    // show help if requested
      "2.0");  // version string

  std::string instruments_file = args["<instruments-file>"].asString();
  std::string score_file = args["<score-file>"].asString();
  std::string effect_file;
  if (args["--effect-file"]) {
      effect_file = args["--effect-file"].asString();
  }

  const char *output_wav = args["<output-wav>"].asString().c_str();

  int verbose = (args["--verbose"].asBool() ? PRINT_SCORE : 0);

  float bpm = stof(args["--bpm"].asString());
  float tpb = stof(args["--tpb"].asString());
  float gain = stof(args["--gain"].asString());


  //Set up the orchest
  Orchest orchest(instruments_file, gain);
  if (!orchest.ready()) {
    cerr << "Error loading instruments from file: " << instruments_file << endl;
    return -2;
  }

  if (not effect_file.empty() && orchest.register_effects(effect_file) < 0) {
    cerr << "Error loading effects from file: " << effect_file << endl;
    return -2;
  }

  //Read score
  vector<MidiMsg> score;
  string s(score_file);
  if (read_midi_score(s, score)) {
    cerr << argv[0] << ": Error reading score file: " << score_file << endl;
    return -3;
  }

  warn_unregistered_instruments(score, orchest, cout);
  warn_unregistered_effects(score, orchest, cout);
 
  //Open output wave file
  SF_INFO fileInfo;
  fileInfo.channels = 1;
  fileInfo.samplerate = SamplingRate;
  fileInfo.format = SF_FORMAT_WAV| SF_FORMAT_PCM_16;    
  SNDFILE* soundFile = sf_open(output_wav, SFM_WRITE, &fileInfo);
  if(soundFile == 0){
    cerr << "Error opening file " << output_wav << endl;
    return -4;
  }

  //Sequencer
  float t = 0; //real time, generated and saved
  float step_t = (float) BSIZE/ (float)SamplingRate; //Duration of BSIZE samples

  float t_score = 0; //time according to the score and tempo (beats-per-second, bps)
  float ticks2seconds = 60.0/ (tpb * bpm); //Duration of one tick in the score
  unsigned long nticks = 0; 

  for (unsigned int i=0; i < score.size(); ++i) {
    if (verbose & PRINT_SCORE)
      cout << score[i] << endl;

    nticks += score[i].getTick();
    t_score = nticks * ticks2seconds;

    if (score[i].getEvent() == 255) {
      if (score[i].getParam1() == 1) {
	      //from microseconds in one quater note => bpm
	      bpm = 1e6/60.0/score[i].getParam2();
	      ticks2seconds = 60.0/ (tpb * bpm); //Duration of one tick in the score
      } else if (score[i].getParam2() == 2) {
	      tpb = score[i].getParam2();
	      ticks2seconds = 60.0/ (tpb * bpm); //Duration of one tick in the score
      }
      continue;
    }

    //Audio synthesis till we arrive to next time in the score
    while (t < t_score) {
      //cout << "synth:\t" << t << '\t' << t_score << endl;

      const vector<float> &xt = orchest.synthesize();
      if (BSIZE !=  sf_writef_float(soundFile, xt.data(), BSIZE)) {
	      cerr << "Error writting file " << output_wav << endl;
	      return -2;
      }
      t += step_t;      
    }

    orchest.command(score[i].getChannel(), score[i].getEvent(), score[i].getParam1(), score[i].getParam2());
  }

  sf_close(soundFile);
  return 0;
}


// Warn if any instrument in the score is not registered
void warn_unregistered_instruments(const vector<MidiMsg> &score, 
				   const Orchest &orchest, ostream &os) {
  set<int> channels;
  //Collect channels
  for (unsigned int i = 0; i < score.size(); ++i)
    channels.insert(score[i].getChannel());

  //Check for unregistered and warn
  set<int>::const_iterator it;
  for (it = channels.begin(); it != channels.end(); ++it) {
    if (!orchest.is_registered(*it))
      os << "Warning: No instrument registered for channel " << *it << '\n';
  }
}

// Warn if any effect in the score is not registered
void warn_unregistered_effects(const vector<MidiMsg> &score, const Orchest &orchest, ostream &os) {
  set<int> effects;
  //Collect channels
  for (unsigned int i = 0; i < score.size(); ++i)
    if (score[i].getEvent() == 12) effects.insert(score[i].getParam1());

  //Check for unregistered and warn
  set<int>::const_iterator it;
  for (it = effects.begin(); it != effects.end(); ++it) {
    if (!orchest.effect_registered(*it))
      os << "Warning: No effect registered for number " << *it << '\n';
  }
}
