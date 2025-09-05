cmake_minimum_required(VERSION 3.10)
project(MySDL2Project C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Hardcoded include paths (Homebrew)
include_directories(
    /opt/homebrew/include
    /opt/homebrew/include/SDL2
)

# Hardcoded library path
link_directories(/opt/homebrew/lib)

# Source files
file(GLOB SRC_FILES src/*.c)

# Build executable
add_executable(my_sdl_app ${SRC_FILES})

# Link SDL2 and SDL2_image
target_link_libraries(my_sdl_app PRIVATE SDL2 SDL2_image)

# Custom target to run the executable with working directory set to project root
add_custom_target(run
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/my_sdl_app
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS my_sdl_app
)

