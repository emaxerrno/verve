cmake_minimum_required(VERSION 3.3)
project(verve)

SET(SMF_ENABLE_UNIT_TESTS OFF CACHE BOOL
  "Disable smf unit tests")
SET(SMF_ENABLE_INTEGRATION_TESTS OFF CACHE BOOL
  "Disable build smf integration tests")
SET(SMF_ENABLE_BENCHMARK_TESTS OFF CACHE BOOL
  "Disable build smf benchmarks")
add_subdirectory(smf)

smf_generate_cpp(verve_fbs verve.fbs)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  # hacks below
  ${PROJECT_SOURCE_DIR}/smf/src
  ${PROJECT_SOURCE_DIR}/smf/src/third_party/flatbuffers/include
  # hacks for out of tree builds
  ${PROJECT_BINARY_DIR}/smf/src
  ${PROJECT_BINARY_DIR}/smf/src/third_party/flatbuffers/include)

add_executable(verved server.cc ${verve_fbs_OUTPUTS})
target_link_libraries(verved smf_rpc)

add_executable(verved_client_tester client.cc ${verve_fbs_OUTPUTS})
target_link_libraries(verved_client_tester smf_rpc)

add_subdirectory(googletest)

add_executable(verve_lock_manager_test
  lock_manager_test.cc)
target_link_libraries(verve_lock_manager_test
  gtest_main)
