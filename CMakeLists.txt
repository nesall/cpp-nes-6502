cmake_minimum_required(VERSION 3.16)
project("cpp-nes-6502" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11) 

set(TARGET_NAME "cppbuild-nes")

if(MSVC)
  message(STATUS "Using MSVC compiler")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /GS")
  if(debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /RTC1")
  endif()  
endif()

include(FetchContent)

#FetchContent_Declare(
#  utils_log
#  GIT_REPOSITORY https://github.com/nesall/utils-log.git
#  GIT_TAG main
#)
#FetchContent_MakeAvailable(utils_log)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

set(HEADER_FILES
  include/nesdefs.hpp
  include/asmemitter.hpp
  include/nesdefs_helper.hpp
)
if(WIN32)
  set(HEADER_FILES ${HEADER_FILES} ./nlohmann_json.natvis)
endif()

# logging
#add_library(utils_log INTERFACE)
#target_include_directories(utils_log INTERFACE ${utils_log_SOURCE_DIR})

# Create executable
add_library(${PROJECT_NAME}
  ${HEADER_FILES}
  src/toolchain.cpp
  src/memorymap.cpp
  src/resources.cpp
  src/program.cpp
  src/subroutine.cpp
  src/rom.cpp
  src/bblocks.cpp
  src/emitter/asmemitter.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME}
  PRIVATE
  include/
)


# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
  #utils_log
  Catch2
)

# ─── CLI Driver (optional, invoke rom.build() from command line) ──────────────
add_executable(${TARGET_NAME} src/main.cpp)
target_include_directories(${TARGET_NAME} PRIVATE include/)
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME})

# Tests
enable_testing()

add_executable(tests
  tests/test_memorymap.cpp
  tests/test_subroutine.cpp
  tests/test_asmemitter.cpp
  tests/test_rom.cpp
)

target_include_directories(tests PRIVATE include/)

target_link_libraries(tests
  PRIVATE
  ${PROJECT_NAME}
  Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(tests)

# ─── Compiler Warnings ────────────────────────────────────────────────────────
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
