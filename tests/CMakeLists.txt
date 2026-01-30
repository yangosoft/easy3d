cmake_minimum_required(VERSION 3.14)
project(easy3d_tests LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
if(NOT TEST_SOURCES)
    message(STATUS "No test sources found in ${CMAKE_CURRENT_SOURCE_DIR}")
    return()
endif()

add_executable(unit_tests ${TEST_SOURCES})
target_compile_features(unit_tests PRIVATE cxx_std_17)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

add_test(NAME unit_tests COMMAND unit_tests)