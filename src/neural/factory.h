#pragma once

#include <functional>
#include <string>
#include "neural/network.h"
#include "neural/loader.h"
#include "utils/optionsdict.h"
#include "utils/optionsparser.h"

namespace pzero {
  
  class NetworkFactory {
  public:
    using FactoryFunc = std::function<std::unique_ptr<Network>
      (/* const WeightsFile&,  */const OptionsDict&)>;

    static NetworkFactory* Get();


    class Register {
    public:
      Register(const std::string& name, FactoryFunc factory, int priority = 0);
    };

    static void PopulateOptions(OptionsParser* options);

    std::unique_ptr<Network> Create
      (const std::string& network,
       /* const WeightsFile&, */
       const OptionsDict& options);

    static std::unique_ptr<Network> LoadNetwork(const OptionsDict& options);

    static const OptionId kWeightsId;
    static const OptionId kBackendId;
    static const OptionId kBackendOptionsId;

  private:
    void RegisterNetwork(const std::string& name,
                         FactoryFunc factory,
                         int priority);

    NetworkFactory() {}

    struct Factory {
      Factory(const std::string& name,
              FactoryFunc factory,
              int priority) {}

      bool operator<(const Factory& other) const {
        if (priority != other.priority) 
          return priority > other.priority;
        return name < other.name;        
      }

      std::string name;
      FactoryFunc factory;
      int priority;
    };

    std::vector<Factory> factories_;
    friend class Register;
  };

#define REGISTER_NETWORK_WITH_COUNTER2(name, func, priority, counter) \
  namespace { \
    static NetworkFactory::Register regH38fs##counter \
      (name, \
      [](const WeightsFile& w, const OptionsDict& o) { return func(w, o); }, \
       priority); \
  }

#define REGISTER_NETWORK_WITH_COUNTER(name, func, priority, counter) \
  REGISTER_NETWORK_WITH_COUNTER2(name, func, priority, counter)

#define REGISTER_NETWORK(name, func, priority) \
  REGISTER_NETWORK_WITH_COUNTER(name, func, priority, __LINE__)
  
} // namespace pzero
