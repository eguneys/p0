#include "uciloop.h"
#include <iomanip>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include "utils/exception.h"
#include "utils/string.h"
#include "version.h"

namespace pzero {
  
  namespace {
    const std::unordered_map<std::string, std::unordered_set<std::string>>
    kKnownCommands = {
      {{"uci"}, {}},
      {{"position"}, {"level"}},
      {{"go"}, { "movetime" }},
      {{"quit"}, {}}
    };
  }


  std::pair<std::string, std::unordered_map<std::string, std::string>>
  ParseCommand(const std::string& line) {
    std::unordered_map<std::string, std::string> params;
    std::string* value = nullptr;

    std::istringstream iss(line);
    std::string token;
    iss >> token >> std::ws;

    // If empty line, return empty command.
    if (token.empty()) return {};

    const auto command = kKnownCommands.find(token);
    if (command == kKnownCommands.end()) {
      throw Exception("Unknown command: " + line);
    }

    std::string whitespace;
    while (iss >> token) {
      auto iter = command->second.find(token);
      if (iter == command->second.end()) {
        if (!value) throw Exception("Unexpected token: " + token);
        *value += whitespace + token;
        whitespace = " ";
      } else {
        value = &params[token];
        iss >> std::ws;
        whitespace = "";
      }
    }
    return {command->first, params};
  }

  std::string GetOrEmpty(const std::unordered_map<std::string, std::string>& params,
                         const std::string& key) {
    const auto iter = params.find(key);
    if (iter == params.end()) return {};
    return iter->second;
  }


  int GetNumeric(const std::unordered_map<std::string, std::string>& params,
                 const std::string& key) {
    const auto iter = params.find(key);
    if (iter == params.end()) {
      throw Exception("Unexpected error");
    }
    const std::string& str = iter->second;
    try {
      if (str.empty()) {
        throw Exception("expected value after " + key);
      }
      return std::stoi(str);
    } catch (std::invalid_argument& e) {
      throw Exception("invalid value " + str);
    }
  }

  bool UciLoop::DispatchCommand(const std::string& command,
                                const std::unordered_map<std::string, std::string>& params) {
    
    if (command == "position") {
      CmdPosition(GetNumeric(params, "level"), {});
    } else if (command == "go") {
      GoParams go_params;
      go_params.movetime = GetNumeric(params, "movetime");
      CmdGo(go_params);
    } else if (command == "quit") {
      return false;
    } else {
      throw Exception("Unknown command: " + command);
    }
    return true;
  }

  void UciLoop::RunLoop() {
    std::cout.setf(std::ios::unitbuf);
    std::string line;
    while (std::getline(std::cin, line)) {
      try {
        auto command = ParseCommand(line);
        if (command.first.empty()) continue;
        if (!DispatchCommand(command.first, command.second)) break;
      } catch (Exception& ex) {
        SendResponse(std::string("error ") + ex.what());
      }
    }
  }

  void UciLoop::SendResponse(const std::string& response) {
    SendResponses({response});
  }

  void UciLoop::SendResponses(const std::vector<std::string>& responses) {
    static std::mutex output_mutex;
    std::lock_guard<std::mutex> lock(output_mutex);
    for (auto& response : responses) {
      std::cout << response << std::endl;
    }
  }

  void UciLoop::SendBestMove(const BestMoveInfo& move) {
    std::string res = "bestmove " + move.bestmove.as_string();

    SendResponse(res);
  }
  
} // namespace pzero
