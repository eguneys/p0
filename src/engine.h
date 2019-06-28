#pragma once

#include "soko/uciloop.h"
#include "mcts/search.h"
#include "neural/factory.h"
#include "neural/network.h"
#include "utils/mutex.h"
#include "utils/optional.h"
#include "utils/optionsparser.h"

namespace pzero {

  struct CurrentPosition {
    std::string fen;
    std::vector<std::string> moves;
  };

  class EngineController {
  public:
    EngineController(const OptionsDict& options);

    ~EngineController() {
      search_.reset();
    }

    void PopulateOptions(OptionsParser* options);

    void SetPosition(const std::string& fen,
                     const std::vector<std::string>& moves);

    void Go(const GoParams& params);

    SearchLimits PopulateSearchLimits(const GoParams& params,
                                      std::chrono::steady_clock::time_point start_time);

  private:
    void UpdateFromUciOptions();

    void SetupPosition(const std::string& fen,
                       const std::vector<std::string>& moves);

    const OptionsDict& options_;

    RpSharedMutex busy_mutex_;
    using SharedLock = std::shared_lock<RpSharedMutex>;

    std::unique_ptr<Search> search_;
    std::unique_ptr<NodeTree> tree_;
    std::unique_ptr<Network> network_;


    optional<CurrentPosition> current_position_;
    GoParams go_params_;

    std::chrono::steady_clock::time_point move_start_time_;
  };

  class EngineLoop : public UciLoop {
  public:
    EngineLoop();

    void RunLoop() override;
    void CmdPosition(const int level,
                     const std::vector<std::string>& moves) override;
    void CmdGo(const GoParams& params) override;

  private:
    OptionsParser options_;
    EngineController engine_;
  };

}
