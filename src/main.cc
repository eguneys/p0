#include "engine.h"
#include "selfplay/loop.h"
#include "utils/logging.h"
#include "utils/commandline.h"
#include "version.h"


int main(int argc, const char** argv) {

  CERR << "     _";
  CERR << " _  | |";
  CERR << "|_| | |";
  CERR << "|   |_| v" << GetVersionStr() << " built " << __DATE__;

  using namespace pzero;
  
  CommandLine::Init(argc, argv);
  CommandLine::RegisterMode("uci", "(default) Act as UCI engine");
  CommandLine::RegisterMode("selfplay", "Play a game with best moves");
  
  if (CommandLine::ConsumeCommand("selfplay")) {
    SelfPlayLoop loop;
    loop.RunLoop();
  } else {
  CommandLine::ConsumeCommand("uci");
  EngineLoop loop;
  loop.RunLoop();
  }
}
