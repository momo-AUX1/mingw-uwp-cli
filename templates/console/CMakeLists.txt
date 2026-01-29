cmake_minimum_required(VERSION 3.16)
project(__PROJECT_NAME__)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
set(MINGW_USE_WINRT ON)
set(MINGW_WINRT_FORCE_FROZEN_SDK ON)
FetchContent_Declare(MinGWWinRT GIT_REPOSITORY https://github.com/momo-AUX1/cmake-mingw-winrt.git GIT_TAG main)
FetchContent_MakeAvailable(MinGWWinRT)

if(DEFINED MinGWWinRT_SOURCE_DIR AND NOT "${MinGWWinRT_SOURCE_DIR}" STREQUAL "")
  include("${MinGWWinRT_SOURCE_DIR}/MinGWWinRT.cmake")
else()
  message(FATAL_ERROR "MinGWWinRT not found. Ensure network access for FetchContent.")
endif()

add_executable(${PROJECT_NAME}
  main.cpp
)
