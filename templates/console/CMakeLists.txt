cmake_minimum_required(VERSION 3.16)
project(__PROJECT_NAME__)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(MINGW_WINRT_COROUTINE_ALIAS_FIX "Patch C++/WinRT coroutine alias for GCC/MinGW on non-Windows hosts" OFF)

include(FetchContent)
set(MINGW_USE_WINRT ON)
set(MINGW_WINRT_FORCE_FROZEN_SDK ON)
FetchContent_Declare(MinGWWinRT GIT_REPOSITORY https://github.com/momo-AUX1/cmake-mingw-winrt.git GIT_TAG main)
FetchContent_MakeAvailable(MinGWWinRT)

if(DEFINED MinGWWinRT_SOURCE_DIR AND NOT "${MinGWWinRT_SOURCE_DIR}" STREQUAL "")
  include("${MinGWWinRT_SOURCE_DIR}/MinGWWinRT.cmake")
    if(MINGW_WINRT_COROUTINE_ALIAS_FIX AND NOT CMAKE_HOST_WIN32)
    set(_winrt_header "${CMAKE_BINARY_DIR}/_deps/mingwwinrt-src/winrt/include/winrt/Windows.Foundation.h")
    if(EXISTS "${_winrt_header}")
      file(READ "${_winrt_header}" _winrt_contents)
      if(_winrt_contents MATCHES "using coroutine_handle = impl::coroutine_handle<>;"
         AND NOT _winrt_contents MATCHES "using coroutine_handle_t = impl::coroutine_handle<>;")
        string(REPLACE "using coroutine_handle = impl::coroutine_handle<>;"
                       "using coroutine_handle_t = impl::coroutine_handle<>;"
                       _winrt_contents "${_winrt_contents}")
        string(REPLACE "coroutine_handle handle" "coroutine_handle_t handle" _winrt_contents "${_winrt_contents}")
        string(REPLACE "coroutine_handle resume" "coroutine_handle_t resume" _winrt_contents "${_winrt_contents}")
        string(REPLACE "coroutine_handle m_handle" "coroutine_handle_t m_handle" _winrt_contents "${_winrt_contents}")
        file(WRITE "${_winrt_header}" "${_winrt_contents}")
        message(STATUS "Applied WinRT coroutine alias workaround for MinGW GCC.")
      endif()
    endif()
  endif()
else()
  message(FATAL_ERROR "MinGWWinRT not found. Ensure network access for FetchContent.")
endif()

add_executable(${PROJECT_NAME}
  main.cpp
)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/include")
  target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/deps/include")
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/bin")
  file(GLOB _deps_bins "${CMAKE_CURRENT_LIST_DIR}/deps/bin/*")
  if(_deps_bins)
    file(COPY ${_deps_bins} DESTINATION ${CMAKE_BINARY_DIR})
  endif()
endif()
