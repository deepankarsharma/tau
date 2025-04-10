cmake_minimum_required(VERSION 3.28)

# Set the project name
project(MainProject C)

# Add executable target
add_executable(main main.c)

# Set C standard (optional, but recommended)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Optional: Add compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(main PRIVATE -O0 -g -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(main PRIVATE /W4)
endif()

target_include_directories(main PRIVATE /home/dman/filc/pizfix/stdfil-include)

# Optional: Installation rules
install(TARGETS main DESTINATION bin)
