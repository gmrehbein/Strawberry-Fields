# CMakeLists.txt for Strawberry Fields (Modernized LLVM setup on macOS and Linux)

cmake_minimum_required(VERSION 3.20)
# Suppress CMP0167 warning about FindBoost deprecation
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(StrawberryFields LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Linux-specific setup (macOS should use toolchain file)
if(UNIX AND NOT APPLE)
  find_program(SYSTEM_CLANG NAMES clang++)
  if(SYSTEM_CLANG)
    message(STATUS "Using system Clang++: ${SYSTEM_CLANG}")
    set(CMAKE_CXX_COMPILER ${SYSTEM_CLANG})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

# Source files
set(SOURCES
  main.cc
  global.cc
  rectangle.cc
  shade.cc
  optimizer.cc
)

# Header files
set(HEADERS
  global.h
  rectangle.h
  shade.h
  optimizer.h
)

# Executable
add_executable(strawberryfields ${SOURCES} ${HEADERS})

# Compiler warnings and optimizations
target_compile_options(strawberryfields PRIVATE
  $<$<CXX_COMPILER_ID:Clang,AppleClang>:
    -Wall -Wextra -Wpedantic -Wconversion -Wno-unused-parameter>
  $<$<CXX_COMPILER_ID:GNU>:
    -Wall -Wextra -Wpedantic -Wconversion -Wno-unused-parameter>
  $<$<CXX_COMPILER_ID:MSVC>:
    /W4 /permissive->
  $<$<CONFIG:Release>:-O3>
  $<$<CONFIG:Debug>:-O0 -g>
)

# Boost dependency
find_package(Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(strawberryfields PRIVATE ${Boost_LIBRARIES})
endif()

# clang-tidy integration (optional)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set_target_properties(
    strawberryfields PROPERTIES
    CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=*,-clang-analyzer-alpha.*"
  )
endif()
