#include "LocalBackend.hpp"

#include <gtest/gtest.h>

TEST(LocalBackendTests, GateDecisionYes) {
  app::LocalBackend local;
  auto backend = local.Backend();
  std::vector<deepseek::Message> messages{
      {"user", "Rule: Allow only software engineering topics.\nInput: C++ agents\n", ""}};

  auto resp = backend.chat(messages, "You are a strict logic gate. Output YES or NO only.", nullptr);
  ASSERT_TRUE(resp.has_value());
  EXPECT_EQ(resp->content, "YES");
}

TEST(LocalBackendTests, GateDecisionNo) {
  app::LocalBackend local;
  auto backend = local.Backend();
  std::vector<deepseek::Message> messages{
      {"user", "Rule: Allow only software engineering topics.\nInput: cooking recipes\n", ""}};

  auto resp = backend.chat(messages, "You are a strict logic gate. Output YES or NO only.", nullptr);
  ASSERT_TRUE(resp.has_value());
  EXPECT_EQ(resp->content, "NO");
}
