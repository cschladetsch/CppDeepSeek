#pragma once

#include <optional>
#include <string>

namespace app {

struct CliOptions {
  std::string topic = "Is C++ the future of AI agents? Argue your position.";
  std::string model = "deepseek-reasoner";
  int rounds = 1;
  bool stream = true;
  bool help = false;
  bool local_only = true;
  std::string load_path;
  std::string save_path;
};

std::string Usage();

std::optional<CliOptions> ParseCli(int argc, char** argv, std::string* error_out = nullptr);

}  // namespace app
