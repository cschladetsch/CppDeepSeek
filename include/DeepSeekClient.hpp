#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace deepseek {

struct Message {
  std::string role;
  std::string content;
  std::string reasoning;
};

struct ChatResponse {
  std::string reasoning;
  std::string content;
  std::string raw;
  long http_status = 0;
};

class DeepSeekClient {
 public:
  using StreamCallback =
      std::function<void(std::string_view reasoning_delta, std::string_view content_delta)>;

  DeepSeekClient(std::string api_key,
                 std::string model = "deepseek-reasoner",
                 std::string base_url = "https://api.deepseek.com");

  void set_timeout_ms(long timeout_ms);

  std::optional<ChatResponse> chat(const std::vector<Message>& messages,
                                   std::string_view system_prompt,
                                   std::string* error_out = nullptr) const;

  bool stream_chat(const std::vector<Message>& messages,
                   std::string_view system_prompt,
                   const StreamCallback& on_delta,
                   std::string* error_out = nullptr) const;

 private:
  std::string api_key_;
  std::string model_;
  std::string base_url_;
  long timeout_ms_ = 30000;
};

}  // namespace deepseek
