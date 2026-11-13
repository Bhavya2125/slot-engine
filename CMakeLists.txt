cmake_minimum_required(VERSION 3.25)
project(slot_engine VERSION 0.1.0 DESCRIPTION "Data-driven slot machine engine" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SLOT_BUILD_TESTS "Build the GoogleTest suite" ON)
option(SLOT_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(SLOT_WERROR "Treat compiler warnings as errors" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ---- Dependencies -----------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  SYSTEM
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(nlohmann_json)

# ---- Warnings and sanitizers --------------------------------------------------
add_library(slot_warnings INTERFACE)
target_compile_options(slot_warnings INTERFACE
  -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
  -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual
  $<$<BOOL:${SLOT_WERROR}>:-Werror>)

if(SLOT_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
  add_link_options(-fsanitize=address,undefined)
endif()

# ---- Core library -------------------------------------------------------------
add_library(slot_core
  src/ReelStrip.cpp
  src/GameConfig.cpp
  src/Evaluator.cpp
  src/SlotMachine.cpp
  src/Simulator.cpp)
target_include_directories(slot_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(slot_core
  PRIVATE nlohmann_json::nlohmann_json slot_warnings)

# ---- Command-line app ---------------------------------------------------------
add_executable(slot app/main.cpp)
target_link_libraries(slot PRIVATE slot_core slot_warnings)
target_compile_definitions(slot PRIVATE
  SLOT_DEFAULT_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/config/classic_5x3.json")

# ---- Tests --------------------------------------------------------------------
if(SLOT_BUILD_TESTS)
  enable_testing()
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    SYSTEM
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  add_subdirectory(tests)
endif()
