#pragma once

#include "DeepSeekClient.hpp"

#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace app {

struct Agent {
  std::string name;
  std::string system_prompt;
  std::vector<deepseek::Message> memory;
};

struct AgentResult {
  std::string name;
  deepseek::ChatResponse response;
};

struct ChatBackend {
  using StreamCallback =
      std::function<void(std::string_view reasoning_delta, std::string_view content_delta)>;
  std::function<std::optional<deepseek::ChatResponse>(
      const std::vector<deepseek::Message>&, std::string_view, std::string*)>
      chat;
  std::function<bool(const std::vector<deepseek::Message>&,
                     std::string_view,
                     const StreamCallback&,
                     std::string*)>
      stream;
};

std::vector<deepseek::Message> BuildPrompt(const Agent& agent, std::string_view user_input);

AgentResult RunAgent(ChatBackend& backend,
                     Agent& agent,
                     std::string_view user_input,
                     bool stream,
                     std::mutex* print_mutex);

std::vector<AgentResult> RunAgentsConcurrent(ChatBackend& backend,
                                             std::vector<Agent>& agents,
                                             std::string_view user_input,
                                             bool stream);

std::vector<AgentResult> RunDebateRounds(ChatBackend& backend,
                                         std::vector<Agent>& agents,
                                         std::string_view topic,
                                         int rounds,
                                         bool stream);

bool SaveAgents(const std::vector<Agent>& agents,
                std::string_view path,
                std::string* error_out = nullptr);

bool LoadAgents(std::vector<Agent>* agents,
                std::string_view path,
                std::string* error_out = nullptr);

}  // namespace app
