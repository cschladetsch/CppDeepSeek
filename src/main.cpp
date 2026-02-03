#include "AgentRuntime.hpp"
#include "AgentRuntime.hpp"
#include "CliOptions.hpp"
#include "DeepSeekClient.hpp"
#include "LogicGate.hpp"
#include "LocalBackend.hpp"
#include "ModelStore.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  std::string parse_error;
  auto options = app::ParseCli(argc, argv, &parse_error);
  if (!options) {
    std::cerr << parse_error << "\n\n" << app::Usage();
    return 1;
  }
  if (options->help) {
    std::cout << app::Usage();
    return 0;
  }

  app::ChatBackend backend;
  std::unique_ptr<deepseek::DeepSeekClient> client;
  std::unique_ptr<app::LocalBackend> local_backend;
  if (options->local_only) {
    local_backend = std::make_unique<app::LocalBackend>();
    backend = local_backend->Backend();
  } else {
    const char* api_key = std::getenv("DEEPSEEK_API_KEY");
    if (!api_key || std::string(api_key).empty()) {
      std::cerr << "Set DEEPSEEK_API_KEY in your environment.\n";
      return 1;
    }
    client = std::make_unique<deepseek::DeepSeekClient>(api_key, options->model);
    backend.chat = [&](const std::vector<deepseek::Message>& messages,
                       std::string_view system_prompt,
                       std::string* error_out) {
      return client->chat(messages, system_prompt, error_out);
    };
    backend.stream = [&](const std::vector<deepseek::Message>& messages,
                         std::string_view system_prompt,
                         const app::ChatBackend::StreamCallback& on_delta,
                         std::string* error_out) {
      return client->stream_chat(messages, system_prompt, on_delta, error_out);
    };
  }

  app::Agent researcher{
      "Researcher",
      "You are a research-oriented agent. Provide evidence, tradeoffs, and cite real engineering"
      " constraints. Be concise.",
      {}};
  app::Agent critic{
      "Critic",
      "You are a critical agent. Challenge assumptions, probe weaknesses, and seek counterexamples."
      " Be concise.",
      {}};

  std::vector<app::Agent> agents{researcher, critic};
  if (!options->load_path.empty()) {
    std::string load_error;
    if (!app::LoadAgents(&agents, options->load_path, &load_error)) {
      std::cerr << "Failed to load agents: " << load_error << "\n";
      return 1;
    }
  }

  const std::string topic = options->topic;
  std::cout << "Debate topic: " << topic << "\n";
  std::cout << "Model: " << options->model << "\n";
  std::cout << "Rounds: " << options->rounds << "\n";
  std::cout << "Streaming: " << (options->stream ? "on" : "off") << "\n";
  std::cout << "Model home (shared across projects): "
            << deepseek::ModelStore::ResolveModelHome() << "\n";
  std::cout << "Example model path (deepseek-r1): "
            << deepseek::ModelStore::ResolveModelPath("deepseek-r1") << "\n";
  if (!deepseek::ModelStore::ModelExists("deepseek-r1")) {
    std::cout << "Model not present. You can place it at: "
              << deepseek::ModelStore::ResolveModelPath("deepseek-r1") << "\n";
  }

  app::LogicGate gate("Allow only software engineering topics.");
  std::string gate_error;
  auto gate_result = gate.Evaluate(backend, topic, false, &gate_error);
  if (!gate_result) {
    std::cerr << "Gate evaluation failed: " << gate_error << "\n";
    return 1;
  }
  if (!gate_result->allow) {
    std::cerr << "Gate rejected the topic.\n";
    return 1;
  }

  try {
    auto results = app::RunDebateRounds(backend, agents, topic, options->rounds, options->stream);
    std::cout << "\n\n--- Summary ---\n";
    for (const auto& result : results) {
      std::cout << result.name << ":\n";
      std::cout << result.response.content << "\n\n";
    }
    if (!options->save_path.empty()) {
      std::string save_error;
      if (!app::SaveAgents(agents, options->save_path, &save_error)) {
        std::cerr << "Failed to save agents: " << save_error << "\n";
        return 1;
      }
    }
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}
