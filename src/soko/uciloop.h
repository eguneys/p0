#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "utils/exception.h"


namespace pzero {
  
  struct GoParams {
    std::int64_t movetime;
  };

  class UciLoop {
  public:
    virtual ~UciLoop() {}
    virtual void RunLoop();

    void SendResponse(const std::string& response);

    virtual void SendResponses(const std::vector<std::string>& responses);
    
    virtual void CmdPosition(const int,
                             const std::vector<std::string>&) {
      throw Exception("Not supported");
    }

    virtual void CmdGo(const GoParams&) {
      throw Exception("Not supported");
    }


    virtual void CmdPlay(const GoParams&) {
      throw Exception("Not supported");
    }

  private:
    bool DispatchCommand(const std::string& command,
                         const std::unordered_map<std::string, std::string>& params);
  };

} //namespace pzero
