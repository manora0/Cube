cmake_minimum_required(VERSION 3.10)

project(cube)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
  include(FetchContent)
  FetchContent_Declare(
          raylib
          GIT_REPOSITORY https://github.com/raysan5/raylib.git
          GIT_TAG 5.5
      )
  FetchContent_MakeAvailable(raylib)
endif()

add_executable(cube src/main.cpp)

target_link_libraries(cube PRIVATE raylib X11)
