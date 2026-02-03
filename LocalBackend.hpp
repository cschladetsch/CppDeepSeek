#pragma once

#include "AgentRuntime.hpp"

#include <string>

namespace app {

class LocalBackend {
 public:
  LocalBackend();

  ChatBackend Backend();

 private:
  static std::string BuildLocalReply(std::string_view system_prompt, std::string_view user_input);
};

}  // namespace app
