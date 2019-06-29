#include "engine.h"
#include "mcts/search.h"

namespace pzero {
  
  namespace {
    const int kDefaultThreads = 2;

    const OptionId kThreadsOptionId{"threads", "Threads",
      "Number of (CPU) worker threads to use.", 't' };
    

  } // namespace

  EngineController::EngineController(const OptionsDict& options)
    :options_(options),
     move_start_time_(std::chrono::steady_clock::now()) {}


  void EngineController::PopulateOptions(OptionsParser* options) {
    options->Add<IntOption>(kThreadsOptionId, 1, 128) = kDefaultThreads;

    SearchParams::Populate(options);
  }

  SearchLimits EngineController::PopulateSearchLimits(const GoParams& params,
                                                      std::chrono::steady_clock::time_point start_time) {
    SearchLimits limits;

    const int64_t time = params.movetime;

    limits.search_deadline =
      start_time +
      std::chrono::milliseconds(time);

    return limits;
  }


  void EngineController::UpdateFromUciOptions() {
    SharedLock lock(busy_mutex_);

    network_ = NetworkFactory::LoadNetwork(options_);
  }

  void EngineController::SetupPosition(const std::string& fen, const std::vector<std::string>& moves_str) {
    SharedLock lock(busy_mutex_);

    search_.reset();

    UpdateFromUciOptions();

    if (!tree_) tree_ = std::make_unique<NodeTree>();

    std::vector<Move> moves;

    for (const auto& move : moves_str) moves.emplace_back(move);
  }

  void EngineController::Go(const GoParams& params) {
    const auto start_time = move_start_time_;
    go_params_ = params;

    if (current_position_) {
      SetupPosition(current_position_->fen, current_position_->moves);
    } else if (!tree_) {
      SetupPosition(SokoBoard::kStartposFen, {});
    }

    auto limits = PopulateSearchLimits(params, start_time);

    search_ = std::make_unique<Search>(*tree_, network_.get(),
                                       limits, options_);

    search_->StartThreads(options_.Get<int>(kThreadsOptionId.GetId()));
  }

  EngineLoop::EngineLoop() :
    engine_(options_.GetOptionsDict()) {
    engine_.PopulateOptions(&options_);
  }

  void EngineLoop::RunLoop() {
    UciLoop::RunLoop();
  }

  void EngineLoop::CmdPosition(const int level,
                               const std::vector<std::string>& moves) {
  }

  void EngineLoop::CmdGo(const GoParams& params) {
    engine_.Go(params);
  }

}
