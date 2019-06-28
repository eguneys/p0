#include "engine.h"
#include "utils/logging.h"
#include "version.h"


int main(int argc, const char** argv) {

  CERR << "     _";
  CERR << " _  | |";
  CERR << "|_| | |";
  CERR << "|   |_| v" << GetVersionStr() << " built " << __DATE__;

  using namespace pzero;
  
  // CommandLine::Init(argc, argv);
  // CommandLine::RegisterMode("uci", "(default) Act as UCI engine");
  
  // CommandLine::ConsumeCommand("uci");
  

  EngineLoop loop;
  loop.RunLoop();
  
}
