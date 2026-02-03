cmake_minimum_required(VERSION 3.20)
project(CppDeepSeek LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

find_package(nlohmann_json CONFIG QUIET)
if (NOT nlohmann_json_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

add_subdirectory(modelstore)

option(CPPDEEPSEEK_BUILD_APP "Build the CppDeepSeek demo application" ON)
if (CPPDEEPSEEK_BUILD_APP)
  add_executable(CppDeepSeek
    main.cpp
    DeepSeekClient.cpp
    AgentRuntime.cpp
    LogicGate.cpp
    CliOptions.cpp
    LocalBackend.cpp
  )

  target_include_directories(CppDeepSeek PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(CppDeepSeek PRIVATE CURL::libcurl nlohmann_json::nlohmann_json ModelStore::ModelStore)
endif()

option(CPPDEEPSEEK_BUILD_TESTS "Build CppDeepSeek tests" ON)
if (CPPDEEPSEEK_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  include(GoogleTest)
  add_executable(AgentRuntimeTests tests/AgentRuntimeTests.cpp AgentRuntime.cpp)
  target_include_directories(AgentRuntimeTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(AgentRuntimeTests PRIVATE GTest::gtest_main nlohmann_json::nlohmann_json)
  gtest_discover_tests(AgentRuntimeTests)

  add_executable(AgentPersistenceTests tests/AgentPersistenceTests.cpp AgentRuntime.cpp)
  target_include_directories(AgentPersistenceTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(AgentPersistenceTests PRIVATE GTest::gtest_main nlohmann_json::nlohmann_json)
  gtest_discover_tests(AgentPersistenceTests)

  add_executable(LogicGateTests tests/LogicGateTests.cpp LogicGate.cpp)
  target_include_directories(LogicGateTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(LogicGateTests PRIVATE GTest::gtest_main)
  gtest_discover_tests(LogicGateTests)

  add_executable(CliOptionsTests tests/CliOptionsTests.cpp CliOptions.cpp)
  target_include_directories(CliOptionsTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(CliOptionsTests PRIVATE GTest::gtest_main)
  gtest_discover_tests(CliOptionsTests)

  add_executable(LocalBackendTests tests/LocalBackendTests.cpp LocalBackend.cpp)
  target_include_directories(LocalBackendTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(LocalBackendTests PRIVATE GTest::gtest_main)
  gtest_discover_tests(LocalBackendTests)
endif()
