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

find_package(SQLiteCpp QUIET)
if(NOT SQLiteCpp_FOUND)
    include(FetchContent)
    # vendor sqlite3's own amalgamation so no system libsqlite3-dev is required.
    set(SQLITECPP_INTERNAL_SQLITE ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        SQLiteCpp
        GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
        GIT_TAG 3.3.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(SQLiteCpp)
endif()

# source files.
set(SOURCES
    src/main.cpp
    src/game/game.cpp
    src/game/logger.cpp
    src/objects/entities/entity.cpp
    src/objects/entities/player.cpp
    src/objects/entities/enemy.cpp
    src/objects/direction.cpp
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
    src/preload/level_loader.cpp
    src/preload/room_loader.cpp
    src/preload/enemy_catalog.cpp
    src/preload/room_generator.cpp
    src/preload/utils/text.cpp
    src/preload/utils/tile_glyph.cpp
    src/systems/movement/pathfinding.cpp
    src/systems/movement/goal_map_cache.cpp
    src/systems/movement/move_enemy.cpp
    src/systems/movement/move_player.cpp
    src/systems/combat/damage_application.cpp
    src/systems/combat/damage_source.cpp
    src/systems/combat/projectile_movement.cpp
    src/systems/combat/projectile_spawn.cpp
    src/systems/visibility/delta.cpp
    src/systems/visibility/recompute.cpp
    src/systems/visibility/update.cpp
    src/objects/tiles/tile.cpp
    src/objects/weapons/weapon.cpp
    src/objects/weapons/projectile.cpp
    src/objects/room/room.cpp
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
    SQLiteCpp
)

target_include_directories(roguelike SYSTEM PRIVATE
    $<TARGET_PROPERTY:SQLiteCpp,INTERFACE_INCLUDE_DIRECTORIES>
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

# roguelike queries game_data.db at startup, so a release package needs it
# alongside the binary same as assets/.
install(FILES ${CMAKE_SOURCE_DIR}/game_data.db DESTINATION .)

# game_data.db generator executable.
add_executable(generate_db
    src/db/main.cpp
    src/db/generate_database.cpp
)

target_include_directories(generate_db PRIVATE include)

target_link_libraries(generate_db PRIVATE
    SQLiteCpp
)

target_include_directories(generate_db SYSTEM PRIVATE
    $<TARGET_PROPERTY:SQLiteCpp,INTERFACE_INCLUDE_DIRECTORIES>
)

# run generate_db at build time to produce game_data.db from db/schema.sql +
# db/seed_data.sql. roguelike depends on this target since it queries the
# database at startup.
set(GAME_DATA_DB ${CMAKE_SOURCE_DIR}/game_data.db)
add_custom_command(
    OUTPUT ${GAME_DATA_DB}
    COMMAND generate_db ${CMAKE_SOURCE_DIR}/db/schema.sql ${CMAKE_SOURCE_DIR}/db/seed_data.sql ${GAME_DATA_DB}
    DEPENDS generate_db ${CMAKE_SOURCE_DIR}/db/schema.sql ${CMAKE_SOURCE_DIR}/db/seed_data.sql
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating game_data.db from db/schema.sql + db/seed_data.sql"
)
add_custom_target(game_db ALL DEPENDS ${GAME_DATA_DB})

# roguelike queries game_data.db at startup, so it must be built first.
add_dependencies(roguelike game_db)