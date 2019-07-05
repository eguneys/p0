#include "selfplay/loop.h"
#include "selfplay/tournament.h"

namespace pzero {

  SelfPlayLoop::SelfPlayLoop() {}

  SelfPlayLoop::~SelfPlayLoop() {
    if (tournament_) tournament_->Abort();
    if (thread_) thread_->join();
  }

  void SelfPlayLoop::RunLoop() {
    SelfPlayTournament::PopulateOptions(&options_);

    if (!options_.ProcessAllFlags()) return;

    SelfPlayTournament tournament
      (options_.GetOptionsDict(),
       std::bind(&UciLoop::SendBestMove, this, std::placeholders::_1),
       std::bind(&SelfPlayLoop::SendTournament, this, std::placeholders::_1));

    tournament.RunBlocking();
  }

  void SelfPlayLoop::SendTournament(const TournamentInfo& info) {
    const int wins = info.results[0];
    const int loses = info.results[1];

    std::ostringstream oss;

    oss << "tournamentstatus";
    if (info.finished) oss << " final";
    oss << " P1: +" << wins << " -" << loses;

    SendResponse(oss.str());
  }
  
} // namespace pzero
