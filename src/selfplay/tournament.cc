#include "selfplay/tournament.h"
#include "mcts/search.h"
#include "neural/factory.h"
#include "selfplay/game.h"
#include "utils/optionsparser.h"

namespace pzero {

  namespace {
    
    const OptionId kTotalGamesId{"games", "Games", "Number of games to play."};
    const OptionId kParallelGamesId{"parallelism", "Paralellism", "Number of games to play in parallel."};

    const OptionId kThreadsId{"threads", "Threads", "Number of (CPU) worker threads to use for every game.", 't'};

    const OptionId kTrainingId{"training", "Training", "Enables writing training data. The training data is stored into a "
      "temporary subdirectory that the engine creates."};
    
  } // namespace

  void SelfPlayTournament::PopulateOptions(OptionsParser* options) {
    NetworkFactory::PopulateOptions(options);
    options->Add<IntOption>(kThreadsId, 1, 8) = 1;
    SearchParams::Populate(options);

    options->Add<IntOption>(kTotalGamesId, -1, 99999) = 1;
    options->Add<IntOption>(kParallelGamesId, 1, 256) = 8;
    options->Add<BoolOption>(kTrainingId) = false;
  }

  SelfPlayTournament::SelfPlayTournament
  (const OptionsDict& options,
   BestMoveInfo::Callback best_move_info,
   TournamentInfo::Callback tournament_info) 
    : best_move_callback_(best_move_info),
      tournament_callback_(tournament_info),
      kThreads(options.Get<int>(kThreadsId.GetId())),
      kTotalGames(options.Get<int>(kTotalGamesId.GetId())),
      kParallelism(options.Get<int>(kParallelGamesId.GetId())),
      kTraining(options.Get<bool>(kTrainingId.GetId())) {

    network_ = NetworkFactory::LoadNetwork(options);
  }

  void SelfPlayTournament::PlayOneGame(int game_number) {
    CERR << game_number;
  }

  void SelfPlayTournament::Worker() {
    while (true) {
      int game_id;
      {
        Mutex::Lock lock(mutex_);
        if (abort_) break;
        if (kTotalGames != -1 && games_count_ >= kTotalGames) break;
        game_id = games_count_++;
      }
      PlayOneGame(game_id);
    }
  }

  void SelfPlayTournament::StartAsync() {
    Mutex::Lock lock(threads_mutex_);
    while (threads_.size() < kParallelism) {
      threads_.emplace_back([&]() { Worker(); });
    }
  }

  void SelfPlayTournament::RunBlocking() {
    if (kParallelism == 1) {
      Worker();
      Mutex::Lock lock(mutex_);
      if (!abort_) {
        tournament_info_.finished = true;
        tournament_callback_(tournament_info_);
      }
    } else {
      StartAsync();
      Wait();
    }
  }

  void SelfPlayTournament::Wait() {
    {
      Mutex::Lock lock(threads_mutex_);
      while (!threads_.empty()) {
        threads_.back().join();
        threads_.pop_back();
      }
    }
    {
      Mutex::Lock lock(mutex_);
      if (!abort_) {
        tournament_info_.finished = true;
        tournament_callback_(tournament_info_);
      }
    }
  }

  void SelfPlayTournament::Abort() {
    Mutex::Lock lock(mutex_);
    abort_ = true;
    for (auto& game : games_) {
      if (game) game->Abort();
    }
  }

  void SelfPlayTournament::Stop() {
    Mutex::Lock lock(mutex_);
    abort_ = true;    
  }

  SelfPlayTournament::~SelfPlayTournament() {
    Abort();
    Wait();
  }
  
} // namespace pzero
