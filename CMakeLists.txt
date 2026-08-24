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
    src/core/game.cpp
    src/core/logger.cpp
    src/core/render_state_builder.cpp
    src/core/enemy_factory.cpp
    src/core/room_enemy_logic.cpp
    src/objects/entities/entity.cpp
    src/objects/entities/player.cpp
    src/objects/entities/enemy.cpp
    src/objects/fovs/fov.cpp
    src/objects/fovs/ellipse_fov.cpp
    src/io/ui_manager.cpp
    src/io/input/handle_input.cpp
    src/io/output/colors.cpp
    src/io/output/render_stack.cpp
    src/io/output/layers/map_layer.cpp
    src/io/output/layers/entity_layer.cpp
    src/io/output/layers/hud_layer.cpp
    src/io/output/layers/debug_layer.cpp
    src/preload/enemy_catalog.cpp
    src/preload/level_loader.cpp
    src/preload/room_loader.cpp
    src/objects/tiles/tile.cpp
    src/objects/weapons/weapon.cpp
    src/objects/weapons/projectile.cpp
    src/objects/room/room.cpp
    src/world/map/level.cpp
    src/world/systems/pathfinding.cpp
    src/world/systems/goal_map_cache.cpp
)

# executable.
add_executable(roguelike ${SOURCES})

# include directories.
target_include_directories(roguelike PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# third-party include dirs marked SYSTEM so clang-tidy's checks skip
# analyzing them entirely, instead of only hiding their diagnostics.
target_include_directories(roguelike SYSTEM PRIVATE
    ${CURSES_INCLUDE_DIRS}
)

# linked thrid-party libraries.
target_link_libraries(roguelike PRIVATE
    ${CURSES_LIBRARIES}
    nlohmann_json::nlohmann_json
)

target_include_directories(roguelike SYSTEM PRIVATE
    $<TARGET_PROPERTY:nlohmann_json::nlohmann_json,INTERFACE_INCLUDE_DIRECTORIES>
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