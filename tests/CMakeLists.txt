add_executable(slot_tests
  ReelStripTest.cpp
  GameConfigTest.cpp
  EvaluatorTest.cpp
  SlotMachineTest.cpp
  SimulatorTest.cpp)

target_include_directories(slot_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(slot_tests PRIVATE slot_core slot_warnings GTest::gtest_main)
target_compile_definitions(slot_tests PRIVATE
  SLOT_CONFIG_DIR="${PROJECT_SOURCE_DIR}/config")

include(GoogleTest)
gtest_discover_tests(slot_tests DISCOVERY_TIMEOUT 30)
