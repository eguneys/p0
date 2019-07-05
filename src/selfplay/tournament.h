#pragma once

#include <list>
#include "selfplay/game.h"
#include "utils/mutex.h"
#include "utils/optionsdict.h"
#include "utils/optionsparser.h"

namespace pzero {

  class SelfPlayTournament {
  public:
    SelfPlayTournament(const OptionsDict& options,
                       BestMoveInfo::Callback best_move_info,
                       TournamentInfo::Callback tournament_info);

    static void PopulateOptions(OptionsParser* options);

    void StartAsync();

    void RunBlocking();

    void Wait();

    void Abort();

    void Stop();

    ~SelfPlayTournament();

  private:
    void Worker();
    void PlayOneGame(int game_id);

    Mutex mutex_;

    int games_count_ GUARDED_BY(mutex_) = 0;

    bool abort_ GUARDED_BY(mutex_) = false;

    std::list<std::unique_ptr<SelfPlayGame>> games_ GUARDED_BY(mutex_);

    TournamentInfo tournament_info_ GUARDED_BY(mutex_);

    Mutex threads_mutex_;
    std::vector<std::thread> threads_ GUARDED_BY(threads_mutex_);

    std::shared_ptr<Network> network_;

    BestMoveInfo::Callback best_move_callback_;
    TournamentInfo::Callback tournament_callback_;
    const int kThreads;
    const int kTotalGames;
    const size_t kParallelism;
    const bool kTraining;
  };

} // namespace pzero
