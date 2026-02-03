#include "AgentRuntime.hpp"

#include <gtest/gtest.h>

#include <filesystem>

TEST(AgentPersistenceTests, SaveAndLoadRoundTrip) {
  std::vector<app::Agent> agents{
      {"A", "System A", {{"user", "hello", ""}, {"assistant", "hi", "reason"}}},
      {"B", "System B", {}},
  };

  const std::string path = "/tmp/agent_memory_test.json";
  std::string error;
  ASSERT_TRUE(app::SaveAgents(agents, path, &error)) << error;

  std::vector<app::Agent> loaded;
  ASSERT_TRUE(app::LoadAgents(&loaded, path, &error)) << error;

  ASSERT_EQ(loaded.size(), 2u);
  EXPECT_EQ(loaded[0].name, "A");
  EXPECT_EQ(loaded[0].system_prompt, "System A");
  ASSERT_EQ(loaded[0].memory.size(), 2u);
  EXPECT_EQ(loaded[0].memory[0].role, "user");
  EXPECT_EQ(loaded[0].memory[0].content, "hello");
  EXPECT_EQ(loaded[0].memory[1].role, "assistant");
  EXPECT_EQ(loaded[0].memory[1].content, "hi");
  EXPECT_EQ(loaded[0].memory[1].reasoning, "reason");

  std::filesystem::remove(path);
}
