# CMakeLists.txt for Strawberry Fields (Portable for macOS and Linux)

cmake_minimum_required(VERSION 3.20)
# Suppress CMP0167 warning about FindBoost deprecation
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(StrawberryFields LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform detection
if(APPLE)
    set(PLATFORM_NAME "macOS")
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
else()
    set(PLATFORM_NAME "Other")
endif()

message(STATUS "Building for ${PLATFORM_NAME}")

# Compiler setup
if(UNIX AND NOT APPLE)
    # Linux: Try to use Clang if available, fall back to GCC
    find_program(CLANG_EXECUTABLE NAMES clang++)
    if(CLANG_EXECUTABLE AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Found Clang: ${CLANG_EXECUTABLE}")
        set(CMAKE_CXX_COMPILER ${CLANG_EXECUTABLE})
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
  # Optimization flags
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
  $<$<CONFIG:Debug>:-O0 -g>
  $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
  $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
)

# Platform-specific Boost setup
if(APPLE)
    # macOS with Homebrew
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    if(HOMEBREW_PREFIX)
        message(STATUS "Found Homebrew at: ${HOMEBREW_PREFIX}")
        set(BOOST_ROOT ${HOMEBREW_PREFIX})
        set(BOOST_INCLUDEDIR ${HOMEBREW_PREFIX}/include)
        set(BOOST_LIBRARYDIR ${HOMEBREW_PREFIX}/lib)
        # Disable Boost's own CMake config on macOS to avoid conflicts
        set(Boost_NO_BOOST_CMAKE ON)
    endif()
elseif(UNIX)
    # Linux: Try pkg-config first, then standard locations
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(BOOST_PC QUIET boost)
        if(BOOST_PC_FOUND)
            message(STATUS "Found Boost via pkg-config")
            set(BOOST_INCLUDEDIR ${BOOST_PC_INCLUDE_DIRS})
            set(BOOST_LIBRARYDIR ${BOOST_PC_LIBRARY_DIRS})
        endif()
    endif()
    
    # Fallback to common Linux locations
    if(NOT BOOST_PC_FOUND)
        # Try common package manager locations
        if(EXISTS "/usr/include/boost" AND EXISTS "/usr/lib/x86_64-linux-gnu")
            set(BOOST_INCLUDEDIR "/usr/include")
            set(BOOST_LIBRARYDIR "/usr/lib/x86_64-linux-gnu")
        elseif(EXISTS "/usr/local/include/boost")
            set(BOOST_ROOT "/usr/local")
        endif()
    endif()
endif()

# Find Boost
find_package(Boost REQUIRED COMPONENTS program_options)

if(Boost_FOUND)
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    
    # Link Boost
    target_link_libraries(strawberryfields PRIVATE 
        Boost::program_options
    )
else()
    message(FATAL_ERROR "Boost not found. Please install boost-dev package or set BOOST_ROOT")
endif()

# Platform-specific installation instructions
if(NOT Boost_FOUND)
    if(APPLE)
        message(STATUS "On macOS, install Boost with: brew install boost")
    elseif(UNIX)
        message(STATUS "On Ubuntu/Debian, install with: sudo apt install libboost-all-dev")
        message(STATUS "On CentOS/RHEL, install with: sudo yum install boost-devel")
        message(STATUS "On Fedora, install with: sudo dnf install boost-devel")
        message(STATUS "On Arch, install with: sudo pacman -S boost")
    endif()
endif()

# Optional clang-tidy discovery
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    # Optionally enable clang-tidy
    # set_target_properties(strawberryfields PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

# Debug information target
add_custom_target(cmake-debug
    COMMAND ${CMAKE_COMMAND} -E echo "=== Build Configuration ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Platform: ${PLATFORM_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "Compiler: ${CMAKE_CXX_COMPILER_ID}"
    COMMAND ${CMAKE_COMMAND} -E echo "Build type: ${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "Boost root: ${BOOST_ROOT}"
    COMMAND ${CMAKE_COMMAND} -E echo "Boost include: ${BOOST_INCLUDEDIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Boost library: ${BOOST_LIBRARYDIR}"
)