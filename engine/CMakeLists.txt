cmake_minimum_required(VERSION 3.20)
project(fomo_family_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(CURL REQUIRED)
find_package(nlohmann_json 3 REQUIRED)
find_package(Threads REQUIRED)

set(WARN -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Werror)

add_library(fomo_core
  src/core/config.cpp
  src/core/http.cpp
  src/core/parse.cpp
  src/agents/discovery.cpp
  src/agents/market.cpp
  src/agents/filters.cpp
  src/agents/decide.cpp
  src/agents/trading.cpp
  src/app/orchestrator.cpp)
target_include_directories(fomo_core PUBLIC src)
target_compile_options(fomo_core PRIVATE ${WARN})
target_link_libraries(fomo_core PUBLIC CURL::libcurl nlohmann_json::nlohmann_json Threads::Threads)

add_executable(fomo-engine src/main.cpp)
target_compile_options(fomo-engine PRIVATE ${WARN})
target_link_libraries(fomo-engine PRIVATE fomo_core)

enable_testing()
add_executable(fomo-tests tests/test_main.cpp)
target_compile_options(fomo-tests PRIVATE ${WARN})
target_compile_definitions(fomo-tests PRIVATE FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
                                              RISKD="${CMAKE_CURRENT_SOURCE_DIR}/../risk/_build/default/bin/riskd.exe")
target_link_libraries(fomo-tests PRIVATE fomo_core)
add_test(NAME fomo-tests COMMAND fomo-tests)
