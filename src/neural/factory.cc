#include "neural/factory.h"

#include <algorithm>
#include "utils/logging.h"

namespace pzero {

  const OptionId NetworkFactory::kWeightsId{
    "weights", "WeightsFile",
      "Path from which to load network weights.\nSetting it to <autodiscover> "
      "makes it search in ./ and ./weights/ subdirectories for the latest (by "
      "file date) file which looks like weights.",
      'w'};

  const OptionId NetworkFactory::kBackendId{
    "backend", "Backend",
      "Neural network computational backend to use.", 'b'};


  
  NetworkFactory* NetworkFactory::Get() {
    static NetworkFactory factory;
    return &factory;
  }

  NetworkFactory::Register::Register
  (const std::string& name, FactoryFunc factory,
   int priority) {
    NetworkFactory::Get()->RegisterNetwork(name, factory, priority);
  }

  void NetworkFactory::PopulateOptions(OptionsParser* options) {
    
  }

  void NetworkFactory::RegisterNetwork
  (const std::string& name, FactoryFunc factory,
   int priority) {
    factories_.emplace_back(name, factory, priority);
    std::sort(factories_.begin(), factories_.end());
  }

  std::unique_ptr<Network> NetworkFactory::Create
  (const std::string& network,
   // const WeightsFile& weights,
   const OptionsDict& options) {
    CERR << "Creating backend [" << network <<"]...";
    for (const auto& factory : factories_) {
      if (factory.name == network) {
        return factory.factory(/* weights, */ options);
      }
    }
    throw Exception("Unknown backend: " + network);
  }

  std::unique_ptr<Network> NetworkFactory::LoadNetwork
  (const OptionsDict& options) {
    std::string backend = options.Get<std::string>(kBackendId.GetId());

    OptionsDict network_options(&options);

    return NetworkFactory::Get()->Create(backend, network_options);
  }

  
} // namespace pzero
