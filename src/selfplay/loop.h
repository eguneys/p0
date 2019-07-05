#pragma once

#include <thread>
#include "soko/uciloop.h"
#include "selfplay/tournament.h"
#include "utils/optionsparser.h"

namespace pzero {
  
  class SelfPlayLoop : public UciLoop {
  public:
    SelfPlayLoop();
    ~SelfPlayLoop();

    void RunLoop() override;

  private:
    void SendTournament(const TournamentInfo& info);
    
    OptionsParser options_;

    std::unique_ptr<SelfPlayTournament> tournament_;
    std::unique_ptr<std::thread> thread_;
  };
  
} // namespace pzero
