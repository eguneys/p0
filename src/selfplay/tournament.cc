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

    const OptionId kTimeMsId{"movetime", "MoveTime", "Time per move, in milliseconds."};

    const OptionId kTrainingId{"training", "Training", "Enables writing training data. The training data is stored into a "
      "temporary subdirectory that the engine creates."};
    
  } // namespace

  void SelfPlayTournament::PopulateOptions(OptionsParser* options) {
    NetworkFactory::PopulateOptions(options);
    options->Add<IntOption>(kThreadsId, 1, 8) = 1;
    SearchParams::Populate(options);

    options->Add<IntOption>(kTotalGamesId, -1, 99999) = 1;
    options->Add<IntOption>(kParallelGamesId, 1, 256) = 8;
    options->Add<IntOption>(kTimeMsId, -1, 9999999) = -1;
    options->Add<BoolOption>(kTrainingId) = false;
  }

  SelfPlayTournament::SelfPlayTournament
  (const OptionsDict& options,
   BestMoveInfo::Callback best_move_info,
   GameInfo::Callback game_info,
   TournamentInfo::Callback tournament_info) 
    : player_options_(options),
      best_move_callback_(best_move_info),
      game_callback_(game_info),
      tournament_callback_(tournament_info),
      kThreads(options.Get<int>(kThreadsId.GetId())),
      kTotalGames(options.Get<int>(kTotalGamesId.GetId())),
      kParallelism(options.Get<int>(kParallelGamesId.GetId())),
      kTraining(options.Get<bool>(kTrainingId.GetId())) {

    network_ = NetworkFactory::LoadNetwork(options);

    search_limits_.movetime = 
      options.Get<int>(kTimeMsId.GetId());

    if (search_limits_.movetime == -1) {
      throw Exception("Please define --movetime, otherwise it's not clear when to stop search.");
    }
  }

  void SelfPlayTournament::PlayOneGame(int game_number) {

    PlayerOptions options;

    options.network = network_.get();
    options.uci_options = &player_options_;
    options.search_limits = search_limits_;

    options.best_move_callback = [this, game_number](const BestMoveInfo& info) {
                               BestMoveInfo rich_info = info;
                               best_move_callback_(rich_info);
                             };


    std::list<std::unique_ptr<SelfPlayGame>>::iterator game_iter;
    {
      Mutex::Lock lock(mutex_);
      games_.emplace_front
        (std::make_unique<SelfPlayGame>(options));
      game_iter = games_.begin();
    }

    auto& game = **game_iter;

    game.Play(kThreads, kTraining);

    if (game.GetGameResult() != GameResult::UNDECIDED) {
      GameInfo game_info;
      game_info.game_result = game.GetGameResult();
      game_info.game_id = game_number;
      game_info.moves = game.GetMoves();

      if (kTraining) {
        TrainingDataWriter writer(game_number);
        game.WriteTrainingData(&writer);
        writer.Finalize();
        game_info.training_filename = writer.GetFileName();
      }

      game_callback_(game_info);

      {
        Mutex::Lock lock(mutex_);
        int result = game.GetGameResult() == GameResult::WIN ? 0 : 1;
        ++tournament_info_.results[result];
        tournament_callback_(tournament_info_);
      }
    }

    {
      Mutex::Lock lock(mutex_);
      games_.erase(game_iter);
    }
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
