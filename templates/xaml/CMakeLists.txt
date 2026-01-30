cmake_minimum_required(VERSION 3.16)
project(__PROJECT_NAME__)

include(CheckCSourceCompiles)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(MINGW_WINRT_COROUTINE_ALIAS_FIX "Patch C++/WinRT coroutine alias for GCC/MinGW on non-Windows hosts" OFF)

if(MSVC)
  set(CMAKE_CXX_FLAGS "/EHsc")
  set(CMAKE_EXE_LINKER_FLAGS "/APPCONTAINER")
  add_definitions("-DUNICODE" "-D_UNICODE")
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|AMD64")
    set(CMAKE_CXX_FLAGS "-mcx16")
  endif()
  set(CMAKE_EXE_LINKER_FLAGS "-municode -static")

  set(CMAKE_REQUIRED_FLAGS "-Wl,--appcontainer")
  check_c_source_compiles("int wWinMain(void) {return 0;}" LINKER_SUPPORTS_APPCONTAINER_FLAG)
  if(LINKER_SUPPORTS_APPCONTAINER_FLAG)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--appcontainer")
  endif()
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
  set(APPX_ARCHITECTURE "x64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86")
  set(APPX_ARCHITECTURE "x86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
  set(APPX_ARCHITECTURE "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM")
  set(APPX_ARCHITECTURE "arm")
else()
  message(FATAL_ERROR "Invalid architecture ${CMAKE_SYSTEM_PROCESSOR}")
endif()

message(STATUS "Building for ${APPX_ARCHITECTURE}.")

file(COPY Images MainPage.xaml DESTINATION ${CMAKE_BINARY_DIR})
configure_file(AppxManifest.in AppxManifest.xml)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/bin")
  file(GLOB _deps_bins "${CMAKE_CURRENT_LIST_DIR}/deps/bin/*")
  if(_deps_bins)
    file(COPY ${_deps_bins} DESTINATION ${CMAKE_BINARY_DIR})
  endif()
endif()

if(NOT MSVC)
  include(FetchContent)
  set(MINGW_USE_WINRT ON)
  set(MINGW_WINRT_FORCE_FROZEN_SDK ON)
  FetchContent_Declare(MinGWWinRT GIT_REPOSITORY https://github.com/momo-AUX1/cmake-mingw-winrt.git GIT_TAG main)
  FetchContent_MakeAvailable(MinGWWinRT)

  set(_mingw_winrt_hint "")
  if(DEFINED ENV{MINGW_WINRT_DIR})
    set(_mingw_winrt_hint "$ENV{MINGW_WINRT_DIR}")
  endif()
  if(NOT DEFINED MinGWWinRT_SOURCE_DIR OR "${MinGWWinRT_SOURCE_DIR}" STREQUAL "")
    if(NOT "${_mingw_winrt_hint}" STREQUAL "" AND EXISTS "${_mingw_winrt_hint}/MinGWWinRT.cmake")
      set(MinGWWinRT_SOURCE_DIR "${_mingw_winrt_hint}")
    elseif(EXISTS "${CMAKE_BINARY_DIR}/_deps/mingwwinrt-src/MinGWWinRT.cmake")
      set(MinGWWinRT_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/mingwwinrt-src")
    endif()
  endif()

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
endif()

add_executable(${PROJECT_NAME} WIN32
  App.cpp
  MainPage.cpp
)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/include")
  target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/deps/include")
endif()

if(NOT MSVC)
  if(APPX_ARCHITECTURE STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _AMD64 _M_AMD64 _WIN64 _M_X64)
  elseif(APPX_ARCHITECTURE STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _X86 _M_IX86 _WIN32)
  elseif(APPX_ARCHITECTURE STREQUAL "arm64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _ARM64)
  elseif(APPX_ARCHITECTURE STREQUAL "arm")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _ARM)
  endif()
endif()

target_link_libraries(${PROJECT_NAME} windowsapp)

set(SIGNING_CERTIFICATE "" CACHE STRING "Path to a .pfx certificate to sign the Appx (optional)")
set(SIGNING_CERT_PASSWORD "" CACHE STRING "Password for the .pfx certificate (optional)")

find_program(MAKEAPPX_EXE NAMES makeappx.exe makeappx MakeAppx.exe MakeAppx)
find_program(SIGNTOOL_EXE NAMES signtool.exe signtool SignTool.exe SignTool)

if(NOT MAKEAPPX_EXE)
  set(_pf86 "")
  if(NOT "$ENV{ProgramFiles}" STREQUAL "")
    set(_pf_candidate "$ENV{ProgramFiles}")
    if(EXISTS "${_pf_candidate}/Windows Kits/10/bin")
      set(_pf86 "${_pf_candidate}")
    endif()
  endif()
  if(_pf86 STREQUAL "" AND NOT "$ENV{ProgramW6432}" STREQUAL "")
    set(_pf_candidate "$ENV{ProgramW6432}")
    if(EXISTS "${_pf_candidate}/Windows Kits/10/bin")
      set(_pf86 "${_pf_candidate}")
    endif()
  endif()
  if(_pf86 STREQUAL "")
    set(_pf_candidate "C:/Program Files (x86)")
    if(EXISTS "${_pf_candidate}/Windows Kits/10/bin")
      set(_pf86 "${_pf_candidate}")
    endif()
  endif()
  if(NOT _pf86 STREQUAL "")
    set(_win_kits_dir "${_pf86}/Windows Kits/10/bin")
    if(EXISTS "${_win_kits_dir}")
      file(GLOB _versions RELATIVE "${_win_kits_dir}" "${_win_kits_dir}/*")
      foreach(_v ${_versions})
        set(_candidate "${_win_kits_dir}/${_v}/${APPX_ARCHITECTURE}/MakeAppx.exe")
        if(EXISTS "${_candidate}")
          set(MAKEAPPX_EXE "${_candidate}")
          message(STATUS "Found MakeAppx at ${MAKEAPPX_EXE}")
          break()
        endif()
        set(_candidate2 "${_win_kits_dir}/${_v}/x64/MakeAppx.exe")
        if(EXISTS "${_candidate2}")
          set(MAKEAPPX_EXE "${_candidate2}")
          message(STATUS "Found MakeAppx at ${MAKEAPPX_EXE}")
          break()
        endif()
      endforeach()
    endif()
  endif()
endif()

if(NOT SIGNTOOL_EXE)
  set(_pf86 "")
  if(NOT "$ENV{ProgramFiles}" STREQUAL "")
    set(_pf_candidate "$ENV{ProgramFiles}")
    if(EXISTS "${_pf_candidate}/Windows Kits/10/bin")
      set(_pf86 "${_pf_candidate}")
    endif()
  endif()
  if(_pf86 STREQUAL "" AND NOT "$ENV{ProgramW6432}" STREQUAL "")
    set(_pf_candidate "$ENV{ProgramW6432}")
    if(EXISTS "${_pf_candidate}/Windows Kits/10/bin")
      set(_pf86 "${_pf_candidate}")
    endif()
  endif()
  if(_pf86 STREQUAL "")
    set(_pf_candidate "C:/Program Files (x86)")
    if(EXISTS "${_pf_candidate}/Windows Kits/10/bin")
      set(_pf86 "${_pf_candidate}")
    endif()
  endif()
  if(NOT _pf86 STREQUAL "")
    set(_win_kits_dir "${_pf86}/Windows Kits/10/bin")
    if(EXISTS "${_win_kits_dir}")
      file(GLOB _versions RELATIVE "${_win_kits_dir}" "${_win_kits_dir}/*")
      foreach(_v ${_versions})
        set(_candidate "${_win_kits_dir}/${_v}/${APPX_ARCHITECTURE}/signtool.exe")
        if(EXISTS "${_candidate}")
          set(SIGNTOOL_EXE "${_candidate}")
          message(STATUS "Found SignTool at ${SIGNTOOL_EXE}")
          break()
        endif()
        set(_candidate2 "${_win_kits_dir}/${_v}/x64/signtool.exe")
        if(EXISTS "${_candidate2}")
          set(SIGNTOOL_EXE "${_candidate2}")
          message(STATUS "Found SignTool at ${SIGNTOOL_EXE}")
          break()
        endif()
      endforeach()
    endif()
  endif()
endif()

if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(_appx_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
  set(_appx_output_dir "${CMAKE_CURRENT_BINARY_DIR}")
endif()

if(MAKEAPPX_EXE)
  add_custom_target(appx
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/appx/Images"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different MainPage.xaml AppxManifest.xml "${PROJECT_NAME}.exe" "${CMAKE_BINARY_DIR}/appx"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/Images" "${CMAKE_BINARY_DIR}/appx/Images"
    COMMAND powershell.exe -NoProfile -Command "& { & '${MAKEAPPX_EXE}' pack /d '${CMAKE_BINARY_DIR}/appx' /p '${CMAKE_BINARY_DIR}/${PROJECT_NAME}.appx' }"
    DEPENDS "${PROJECT_NAME}.exe"
  )

  if(SIGNTOOL_EXE)
    if(SIGNING_CERTIFICATE)
      if(SIGNING_CERT_PASSWORD)
        add_custom_command(TARGET appx POST_BUILD
          COMMAND ${SIGNTOOL_EXE} sign /fd SHA256 /a /f "${SIGNING_CERTIFICATE}" /p "${SIGNING_CERT_PASSWORD}" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.appx"
        )
      else()
        add_custom_command(TARGET appx POST_BUILD
          COMMAND ${SIGNTOOL_EXE} sign /fd SHA256 /a /f "${SIGNING_CERTIFICATE}" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.appx"
        )
      endif()
    else()
      message(WARNING "SignTool found (${SIGNTOOL_EXE}) but SIGNING_CERTIFICATE not set. Package will not be signed.")
    endif()
  else()
    message(STATUS "MakeAppx found at ${MAKEAPPX_EXE} but SignTool was not found; package will remain unsigned unless you provide SignTool.")
  endif()
else()
  set(TRY_MAKEAPPX "makeappx.exe")
  if(MAKEAPPX_EXE)
    set(TRY_MAKEAPPX "${MAKEAPPX_EXE}")
  endif()

  add_custom_target(appx
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/appx/Images"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different MainPage.xaml AppxManifest.xml "${PROJECT_NAME}.exe" "${CMAKE_BINARY_DIR}/appx"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/Images" "${CMAKE_BINARY_DIR}/appx/Images"
    COMMAND powershell.exe -NoProfile -Command "& { & '${TRY_MAKEAPPX}' pack /d '${CMAKE_BINARY_DIR}/appx' /p '${CMAKE_BINARY_DIR}/${PROJECT_NAME}.appx' }"
    DEPENDS "${PROJECT_NAME}.exe"
  )
endif()

add_custom_command(TARGET appx POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.appx" "${_appx_output_dir}/${PROJECT_NAME}.appx"
)

if(MAKEAPPX_EXE)
  add_custom_target(msix
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/msix/Images"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different MainPage.xaml AppxManifest.xml "${PROJECT_NAME}.exe" "${CMAKE_BINARY_DIR}/msix"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/Images" "${CMAKE_BINARY_DIR}/msix/Images"
    COMMAND powershell.exe -NoProfile -Command "& { & '${MAKEAPPX_EXE}' pack /d '${CMAKE_BINARY_DIR}/msix' /p '${CMAKE_BINARY_DIR}/${PROJECT_NAME}.msix' }"
    DEPENDS "${PROJECT_NAME}.exe"
  )

  if(SIGNTOOL_EXE)
    if(SIGNING_CERTIFICATE)
      if(SIGNING_CERT_PASSWORD)
        add_custom_command(TARGET msix POST_BUILD
          COMMAND ${SIGNTOOL_EXE} sign /fd SHA256 /a /f "${SIGNING_CERTIFICATE}" /p "${SIGNING_CERT_PASSWORD}" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.msix"
        )
      else()
        add_custom_command(TARGET msix POST_BUILD
          COMMAND ${SIGNTOOL_EXE} sign /fd SHA256 /a /f "${SIGNING_CERTIFICATE}" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.msix"
        )
      endif()
    else()
      message(WARNING "SignTool found (${SIGNTOOL_EXE}) but SIGNING_CERTIFICATE not set. MSIX will not be signed.")
    endif()
  else()
    message(STATUS "MakeAppx found at ${MAKEAPPX_EXE} but SignTool was not found; MSIX will remain unsigned unless you provide SignTool.")
  endif()
else()
  set(TRY_MAKEAPPX "makeappx.exe")
  if(MAKEAPPX_EXE)
    set(TRY_MAKEAPPX "${MAKEAPPX_EXE}")
  endif()

  add_custom_target(msix
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/msix/Images"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different MainPage.xaml AppxManifest.xml "${PROJECT_NAME}.exe" "${CMAKE_BINARY_DIR}/msix"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/Images" "${CMAKE_BINARY_DIR}/msix/Images"
    COMMAND powershell.exe -NoProfile -Command "& { & '${TRY_MAKEAPPX}' pack /d '${CMAKE_BINARY_DIR}/msix' /p '${CMAKE_BINARY_DIR}/${PROJECT_NAME}.msix' }"
    DEPENDS "${PROJECT_NAME}.exe"
  )
endif()

add_custom_command(TARGET msix POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.msix" "${_appx_output_dir}/${PROJECT_NAME}.msix"
)
