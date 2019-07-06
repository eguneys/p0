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
       std::bind(&SelfPlayLoop::SendGameInfo, this, std::placeholders::_1),
       std::bind(&SelfPlayLoop::SendTournament, this, std::placeholders::_1));

    tournament.RunBlocking();
  }

  void SelfPlayLoop::SendGameInfo(const GameInfo& info) {
    std::vector<std::string> responses;

    std::string res = "gameready";

    if (!info.training_filename.empty()) {
      res += "trainingfile " + info.training_filename;
    }

    if (info.game_id != -1) res += " gameid " + std::to_string(info.game_id);

    if (info.game_result != GameResult::UNDECIDED) {
      res += std::string(" result ") +
        ((info.game_result == GameResult::WIN) ? "win" : "lose");
    }

    if (!info.moves.empty()) {
      res += " moves";
      for (const auto& move: info.moves) res += " " + move.as_string();
    }
    
    responses.push_back(res);
    
    SendResponses(responses);
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
