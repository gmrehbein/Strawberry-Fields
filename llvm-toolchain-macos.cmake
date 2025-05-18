# llvm-toolchain-macos.cmake

# Use Homebrew's LLVM
set(CMAKE_C_COMPILER /opt/homebrew/opt/llvm/bin/clang)
set(CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++)

# Get the macOS SDK path from xcrun
execute_process(
  COMMAND xcrun --show-sdk-path
  OUTPUT_VARIABLE MACOS_SDK_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set SDK for both compiler and linker
set(CMAKE_SYSROOT "${MACOS_SDK_PATH}")
