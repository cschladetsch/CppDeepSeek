#include "DeepSeekClient.hpp"
#include "DeepSeekStreamParser.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <string>

namespace deepseek {
namespace {

class CurlGlobal {
 public:
  CurlGlobal() { curl_global_init(CURL_GLOBAL_DEFAULT); }
  ~CurlGlobal() { curl_global_cleanup(); }
};

const CurlGlobal kCurlGlobal;

size_t WriteToString(void* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* out = static_cast<std::string*>(userdata);
  out->append(static_cast<const char*>(ptr), size * nmemb);
  return size * nmemb;
}

struct StreamState {
  DeepSeekStreamParser parser;
  std::string* error_out = nullptr;
};

size_t StreamWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* state = static_cast<StreamState*>(userdata);
  const size_t total = size * nmemb;
  if (!state->parser.Feed(std::string_view(static_cast<const char*>(ptr), total),
                          state->error_out)) {
    return 0;
  }
  return total;
}

bool CheckHttpStatus(long status, std::string* error_out) {
  if (status == 200) {
    return true;
  }
  if (!error_out) {
    return false;
  }
  if (status == 401) {
    *error_out = "HTTP 401 Unauthorized (check API key).";
  } else if (status == 429) {
    *error_out = "HTTP 429 Too Many Requests (rate limited).";
  } else if (status >= 500) {
    *error_out = "HTTP 5xx Server Error.";
  } else {
    *error_out = "HTTP error status: " + std::to_string(status);
  }
  return false;
}

}  // namespace

DeepSeekClient::DeepSeekClient(std::string api_key, std::string model, std::string base_url)
    : api_key_(std::move(api_key)),
      model_(std::move(model)),
      base_url_(std::move(base_url)) {}

void DeepSeekClient::set_timeout_ms(long timeout_ms) { timeout_ms_ = timeout_ms; }

std::optional<ChatResponse> DeepSeekClient::chat(const std::vector<Message>& messages,
                                                 std::string_view system_prompt,
                                                 std::string* error_out) const {
  nlohmann::json payload;
  payload["model"] = model_;
  payload["messages"] = nlohmann::json::array();
  payload["messages"].push_back({{"role", "system"}, {"content", system_prompt}});
  for (const auto& msg : messages) {
    payload["messages"].push_back({{"role", msg.role}, {"content", msg.content}});
  }

  std::string response;
  CURL* curl = curl_easy_init();
  if (!curl) {
    if (error_out) {
      *error_out = "Failed to initialize CURL.";
    }
    return std::nullopt;
  }

  const std::string url = base_url_ + "/v1/chat/completions";
  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());

  const std::string payload_str = payload.dump();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms_);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  CURLcode res = curl_easy_perform(curl);
  long status = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    if (error_out) {
      *error_out = std::string("CURL error: ") + curl_easy_strerror(res);
    }
    return std::nullopt;
  }
  if (!CheckHttpStatus(status, error_out)) {
    return std::nullopt;
  }

  nlohmann::json j;
  try {
    j = nlohmann::json::parse(response);
  } catch (const std::exception& ex) {
    if (error_out) {
      *error_out = std::string("Invalid JSON response: ") + ex.what();
    }
    return std::nullopt;
  }

  ChatResponse out;
  out.raw = response;
  out.http_status = status;

  if (j.contains("choices") && !j["choices"].empty()) {
    const auto& message = j["choices"][0].value("message", nlohmann::json::object());
    // DeepSeek-R1 provides "reasoning_content" separate from final "content".
    // We expose them separately so agent logic can keep thoughts (reasoning)
    // distinct from the user-facing output.
    out.reasoning = message.value("reasoning_content", "");
    out.content = message.value("content", "");
  }

  return out;
}

bool DeepSeekClient::stream_chat(const std::vector<Message>& messages,
                                 std::string_view system_prompt,
                                 const StreamCallback& on_delta,
                                 std::string* error_out) const {
  nlohmann::json payload;
  payload["model"] = model_;
  payload["stream"] = true;
  payload["messages"] = nlohmann::json::array();
  payload["messages"].push_back({{"role", "system"}, {"content", system_prompt}});
  for (const auto& msg : messages) {
    payload["messages"].push_back({{"role", msg.role}, {"content", msg.content}});
  }

  StreamState state{DeepSeekStreamParser(on_delta), error_out};

  CURL* curl = curl_easy_init();
  if (!curl) {
    if (error_out) {
      *error_out = "Failed to initialize CURL.";
    }
    return false;
  }

  const std::string url = base_url_ + "/v1/chat/completions";
  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "Accept: text/event-stream");
  headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());

  const std::string payload_str = payload.dump();
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms_);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamWriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &state);

  CURLcode res = curl_easy_perform(curl);
  long status = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    if (error_out && error_out->empty()) {
      *error_out = std::string("CURL error: ") + curl_easy_strerror(res);
    }
    return false;
  }
  return CheckHttpStatus(status, error_out);
}

}  // namespace deepseek
