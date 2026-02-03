#include "LogicGate.hpp"

#include <gtest/gtest.h>

TEST(LogicGateTests, AcceptsYesResponse) {
  app::ChatBackend backend;
  backend.chat = [&](const std::vector<deepseek::Message>&,
                     std::string_view,
                     std::string*) -> std::optional<deepseek::ChatResponse> {
    deepseek::ChatResponse resp;
    resp.content = "YES";
    return resp;
  };
  backend.stream = [&](const std::vector<deepseek::Message>&,
                       std::string_view,
                       const app::ChatBackend::StreamCallback&,
                       std::string*) { return false; };

  app::LogicGate gate("Allow only safe content.");
  auto result = gate.Evaluate(backend, "test input", false);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->allow);
}

TEST(LogicGateTests, RejectsInvalidResponse) {
  app::ChatBackend backend;
  backend.chat = [&](const std::vector<deepseek::Message>&,
                     std::string_view,
                     std::string*) -> std::optional<deepseek::ChatResponse> {
    deepseek::ChatResponse resp;
    resp.content = "MAYBE";
    return resp;
  };
  backend.stream = [&](const std::vector<deepseek::Message>&,
                       std::string_view,
                       const app::ChatBackend::StreamCallback&,
                       std::string*) { return false; };

  app::LogicGate gate("Allow only safe content.");
  std::string error;
  auto result = gate.Evaluate(backend, "test input", false, &error);
  EXPECT_FALSE(result.has_value());
  EXPECT_FALSE(error.empty());
}
