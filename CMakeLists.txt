cmake_minimum_required(VERSION 3.3)
project(verve)

SET(SMF_BUILD_PROGRAMS OFF CACHE BOOL
  "Disable building smf programs")
SET(SMF_ENABLE_UNIT_TESTS OFF CACHE BOOL
  "Disable smf unit tests")
SET(SMF_ENABLE_INTEGRATION_TESTS OFF CACHE BOOL
  "Disable build smf integration tests")
SET(SMF_ENABLE_BENCHMARK_TESTS OFF CACHE BOOL
  "Disable build smf benchmarks")
add_subdirectory(smf)

smfc_gen(
  CPP
  TARGET_NAME verve_fbs
  OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES verve.fbs)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR})

add_executable(verved server.cc ${verve_fbs})
target_link_libraries(verved smf)

add_executable(verved_client_tester client.cc ${verve_fbs})
target_link_libraries(verved_client_tester smf)

add_subdirectory(googletest/googletest)

add_executable(verve_lock_manager_test
  lock_manager_test.cc)
target_link_libraries(verve_lock_manager_test
  gtest_main)
