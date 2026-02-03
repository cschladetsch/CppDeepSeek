#include "AgentRuntime.hpp"

#include <gtest/gtest.h>

TEST(AgentRuntimeTests, MultiTurnDebateUpdatesMemoryAndOrder) {
  std::vector<app::Agent> agents{
      {"Researcher", "Research prompt", {}},
      {"Critic", "Critic prompt", {}},
  };

  std::vector<std::string> outputs{"R1", "C1", "R2", "C2"};
  std::vector<std::string> user_inputs;
  size_t call = 0;

  app::ChatBackend backend;
  backend.chat = [&](const std::vector<deepseek::Message>& messages,
                     std::string_view /*system_prompt*/,
                     std::string* /*error_out*/) -> std::optional<deepseek::ChatResponse> {
    if (messages.empty()) {
      return std::nullopt;
    }
    user_inputs.push_back(messages.back().content);
    deepseek::ChatResponse resp;
    resp.content = outputs.at(call++);
    return resp;
  };
  backend.stream = [&](const std::vector<deepseek::Message>&,
                       std::string_view,
                       const app::ChatBackend::StreamCallback&,
                       std::string*) { return false; };

  auto results = app::RunDebateRounds(backend, agents, "topic", 2, false);

  ASSERT_EQ(results.size(), 4u);
  EXPECT_EQ(results[0].response.content, "R1");
  EXPECT_EQ(results[1].response.content, "C1");
  EXPECT_EQ(results[2].response.content, "R2");
  EXPECT_EQ(results[3].response.content, "C2");

  ASSERT_EQ(user_inputs.size(), 4u);
  EXPECT_EQ(user_inputs[0], "topic");
  EXPECT_EQ(user_inputs[1], "R1");
  EXPECT_EQ(user_inputs[2], "C1");
  EXPECT_EQ(user_inputs[3], "R2");

  EXPECT_EQ(agents[0].memory.size(), 2u);
  EXPECT_EQ(agents[1].memory.size(), 2u);
}
