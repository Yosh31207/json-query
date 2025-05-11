cmake_minimum_required(VERSION 3.15)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(json_query_test CXX)

enable_testing()

find_package(boost_json CONFIG REQUIRED)
find_package(boost_hana CONFIG REQUIRED)
find_package(boost_preprocessor CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_executable(
  json_query_test
  src/json_query_test.cpp
)

target_link_libraries(
  json_query_test
  PRIVATE
  Boost::json
  GTest::gtest
  GTest::gtest_main
)

target_include_directories(
  json_query_test
  PRIVATE
  ../include
)

target_compile_options(
  json_query_test
  PRIVATE
  -Wall
  -Wextra
  -Werror
)

add_test(tests json_query_test)
