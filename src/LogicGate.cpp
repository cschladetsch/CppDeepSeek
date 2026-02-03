#include "LogicGate.hpp"

#include <algorithm>
#include <cctype>

namespace app {
namespace {

std::string BuildGatePrompt(std::string_view rule, std::string_view input) {
  std::string prompt;
  prompt.reserve(rule.size() + input.size() + 64);
  prompt.append("Rule: ").append(rule).append("\n");
  prompt.append("Input: ").append(input).append("\n");
  prompt.append("Answer YES or NO only.");
  return prompt;
}

std::optional<bool> ParseDecision(std::string_view content) {
  size_t i = 0;
  while (i < content.size() && std::isspace(static_cast<unsigned char>(content[i]))) {
    ++i;
  }
  size_t start = i;
  while (i < content.size() && std::isalpha(static_cast<unsigned char>(content[i]))) {
    ++i;
  }
  if (start == i) {
    return std::nullopt;
  }
  std::string token(content.substr(start, i - start));
  std::transform(token.begin(), token.end(), token.begin(),
                 [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
  if (token == "YES") {
    return true;
  }
  if (token == "NO") {
    return false;
  }
  return std::nullopt;
}

}  // namespace

LogicGate::LogicGate(std::string rule) : rule_(std::move(rule)) {}

std::optional<GateResult> LogicGate::Evaluate(ChatBackend& backend,
                                              std::string_view input,
                                              bool stream,
                                              std::string* error_out) const {
  std::vector<deepseek::Message> messages;
  messages.push_back({"user", BuildGatePrompt(rule_, input), ""});

  if (stream) {
    std::string reasoning_accum;
    std::string content_accum;
    bool ok = backend.stream(
        messages, "You are a strict logic gate. Output YES or NO only.",
        [&](std::string_view reasoning_delta, std::string_view content_delta) {
          reasoning_accum.append(reasoning_delta);
          content_accum.append(content_delta);
        },
        error_out);
    if (!ok) {
      return std::nullopt;
    }
    auto decision = ParseDecision(content_accum);
    if (!decision) {
      if (error_out) {
        *error_out = "Gate did not return YES/NO.";
      }
      return std::nullopt;
    }
    return GateResult{*decision, content_accum, reasoning_accum};
  }

  auto resp = backend.chat(messages, "You are a strict logic gate. Output YES or NO only.",
                           error_out);
  if (!resp) {
    return std::nullopt;
  }
  auto decision = ParseDecision(resp->content);
  if (!decision) {
    if (error_out) {
      *error_out = "Gate did not return YES/NO.";
    }
    return std::nullopt;
  }
  return GateResult{*decision, resp->content, resp->reasoning};
}

}  // namespace app
