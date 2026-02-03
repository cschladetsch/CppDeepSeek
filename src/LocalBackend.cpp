#include "LocalBackend.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace app {
namespace {

bool ContainsToken(std::string_view text, std::string_view token) {
  auto it = std::search(
      text.begin(), text.end(),
      token.begin(), token.end(),
      [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) ==
                                  std::tolower(static_cast<unsigned char>(b)); });
  return it != text.end();
}

std::string GateDecision(std::string_view input) {
  std::string_view inspect = input;
  auto pos = input.find("Input:");
  if (pos != std::string_view::npos) {
    inspect = input.substr(pos + 6);
  }
  if (ContainsToken(inspect, "c++") || ContainsToken(inspect, "software") ||
      ContainsToken(inspect, "agent") || ContainsToken(inspect, "program")) {
    return "YES";
  }
  return "NO";
}

}  // namespace

LocalBackend::LocalBackend() = default;

ChatBackend LocalBackend::Backend() {
  ChatBackend backend;
  backend.chat = [](const std::vector<deepseek::Message>& messages,
                    std::string_view system_prompt,
                    std::string* /*error_out*/) -> std::optional<deepseek::ChatResponse> {
    if (messages.empty()) {
      return std::nullopt;
    }
    const std::string& user_input = messages.back().content;
    deepseek::ChatResponse resp;
    if (ContainsToken(system_prompt, "logic gate")) {
      resp.content = GateDecision(user_input);
      return resp;
    }
    resp.content = BuildLocalReply(system_prompt, user_input);
    return resp;
  };
  backend.stream = [](const std::vector<deepseek::Message>& messages,
                      std::string_view system_prompt,
                      const ChatBackend::StreamCallback& on_delta,
                      std::string* /*error_out*/) {
    if (messages.empty()) {
      return false;
    }
    const std::string& user_input = messages.back().content;
    std::string text;
    if (ContainsToken(system_prompt, "logic gate")) {
      text = GateDecision(user_input);
    } else {
      text = BuildLocalReply(system_prompt, user_input);
    }
    const size_t chunk = 8;
    for (size_t i = 0; i < text.size(); i += chunk) {
      on_delta("", std::string_view(text.data() + i, std::min(chunk, text.size() - i)));
    }
    return true;
  };
  return backend;
}

std::string LocalBackend::BuildLocalReply(std::string_view system_prompt,
                                          std::string_view user_input) {
  if (ContainsToken(system_prompt, "research")) {
    return "Local Researcher: C++ offers performance and control, but iteration speed and "
           "tooling can slow agent development. A hybrid stack is likely.";
  }
  if (ContainsToken(system_prompt, "critical")) {
    return "Local Critic: C++ may excel at runtime efficiency, but developer velocity and "
           "library ecosystems often favor higher-level languages for agents.";
  }
  return std::string("Local: ") + std::string(user_input);
}

}  // namespace app
