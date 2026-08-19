cmake_minimum_required(VERSION 3.16)

project(Roguelike)

# project config.
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# dependencies.
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)

find_package(nlohmann_json 3.11 QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# source files.
set(SOURCES
    src/main.cpp
    src/core/colors.cpp
    src/core/game.cpp
    src/core/logger.cpp
    src/entities/entity.cpp
    src/entities/player.cpp
    src/entities/enemy.cpp
    src/entities/enemy_catalog.cpp
    src/entities/enemy_factory.cpp
    src/entities/room_enemy_state.cpp
    src/entities/fov.cpp
    src/world/map/tile.cpp
    src/world/map/room.cpp
    src/world/map/level.cpp
    src/world/map/level_config.cpp
    src/world/systems/pathfinding.cpp
    src/world/systems/goal_map_cache.cpp
    src/world/objects/weapon.cpp
    src/world/objects/projectile.cpp
    src/render/renderer.cpp
    src/render/render_stack.cpp
    src/render/layers/map_layer.cpp
    src/render/layers/entity_layer.cpp
    src/render/layers/hud_layer.cpp
    src/render/layers/debug_layer.cpp
)

# executable.
add_executable(roguelike ${SOURCES})

# include directories.
target_include_directories(roguelike PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${CURSES_INCLUDE_DIRS}
)

# linked thrid-party libraries.
target_link_libraries(roguelike PRIVATE
    ${CURSES_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# expose the wide-char ncurses API for macOS/Homebrew builds.
target_compile_definitions(roguelike PRIVATE
    _XOPEN_SOURCE_EXTENDED
)

# compiler warnings.
target_compile_options(roguelike PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# installation.
install(TARGETS roguelike
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    README.md
    DESTINATION .
)

install(DIRECTORY assets DESTINATION .)