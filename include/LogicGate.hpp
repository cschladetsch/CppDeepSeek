#pragma once

#include "AgentRuntime.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace app {

struct GateResult {
  bool allow = false;
  std::string content;
  std::string reasoning;
};

class LogicGate {
 public:
  explicit LogicGate(std::string rule);

  const std::string& rule() const { return rule_; }

  std::optional<GateResult> Evaluate(ChatBackend& backend,
                                     std::string_view input,
                                     bool stream,
                                     std::string* error_out = nullptr) const;

 private:
  std::string rule_;
};

}  // namespace app
