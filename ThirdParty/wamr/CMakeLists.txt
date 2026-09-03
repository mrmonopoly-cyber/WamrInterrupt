cmake_minimum_required (VERSION 3.14)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

if(ESP_PLATFORM)
  include (${COMPONENT_DIR}/build-scripts/esp-idf/wamr/CMakeLists.txt)
  return()
endif()

project (wams-app-launcher LANGUAGES C)

set(CMAKE_CXX_STANDARD 17)

set (CMAKE_VERBOSE_MAKEFILE OFF)

# reset default linker flags
set (cmake_shared_library_link_c_flags "")
set (cmake_shared_library_link_cxx_flags "")

set (cmake_c_standard 99)

# Set WAMR_BUILD_TARGET, currently values supported:
# "X86_64", "AMD_64", "X86_32", "AARCH64[sub]", "ARM[sub]", "THUMB[sub]",
# "MIPS", "XTENSA", "RISCV64[sub]", "RISCV32[sub]"
if (NOT DEFINED WAMR_BUILD_TARGET)
  if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)")
    set (WAMR_BUILD_TARGET "AARCH64")
  elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "riscv64")
    set (WAMR_BUILD_TARGET "RISCV64")
  elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Build as X86_64 by default in 64-bit platform
    set (WAMR_BUILD_TARGET "X86_64")
  elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    # Build as X86_32 by default in 32-bit platform
    set (WAMR_BUILD_TARGET "X86_32")
  else ()
    message(SEND_ERROR "Unsupported build target platform!")
  endif ()
endif ()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
      set (cmake_shared_library_link_c_flags
          -ggdb 
          -fno-omit-frame-pointer
      )
endif ()


include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)

# include bh_read_file.h
include (${WAMR_ROOT_DIR}/core/shared/utils/uncommon/shared_uncommon.cmake)

add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE} ${UNCOMMON_SHARED_SOURCE})
