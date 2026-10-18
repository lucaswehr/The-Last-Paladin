cmake_minimum_required(VERSION 3.26)
project(TheLastPaladin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/2D Sidescroller")

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window Audio Network System)

# ---------------------------------------------------------------------------
# Game client (also runs as a dedicated game server with the "server" arg)
# ---------------------------------------------------------------------------
add_executable(the_last_paladin
    "${SRC_DIR}/Arrow.cpp"
    "${SRC_DIR}/AssetManager.cpp"
    "${SRC_DIR}/Client.cpp"
    "${SRC_DIR}/DeflectionDemo.cpp"
    "${SRC_DIR}/Dragon.cpp"
    "${SRC_DIR}/Enemy.cpp"
    "${SRC_DIR}/Knight.cpp"
    "${SRC_DIR}/Level.cpp"
    "${SRC_DIR}/LobbyClient.cpp"
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/ParryDemo.cpp"
    "${SRC_DIR}/Player.cpp"
    "${SRC_DIR}/Samurai.cpp"
    "${SRC_DIR}/Skeleton.cpp"
    "${SRC_DIR}/text.cpp"
    "${SRC_DIR}/Tile.cpp"
    "${SRC_DIR}/wolf.cpp"
)

target_link_libraries(the_last_paladin PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::Network
    SFML::System
)

# Assets are loaded relative to the working directory, so copy them next to
# the executable and run it from there.
set(ASSET_DIRS
    Backgrounds Dragon Fonts GUI Knight_1 Samurai_Archer Skeleton_Archer
    Sounds Tileset wolf
)
foreach(dir IN LISTS ASSET_DIRS)
    add_custom_command(TARGET the_last_paladin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${SRC_DIR}/${dir}" "$<TARGET_FILE_DIR:the_last_paladin>/${dir}"
        VERBATIM)
endforeach()
add_custom_command(TARGET the_last_paladin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SRC_DIR}/Inventory.png" "$<TARGET_FILE_DIR:the_last_paladin>/Inventory.png"
    VERBATIM)

# `cmake --build build --target run` builds and launches from the right folder
add_custom_target(run
    COMMAND $<TARGET_FILE:the_last_paladin>
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:the_last_paladin>"
    DEPENDS the_last_paladin
    USES_TERMINAL)

# ---------------------------------------------------------------------------
# Lobby server (Azure-hosted; needs libcurl). Off by default for desktop builds.
# ---------------------------------------------------------------------------
option(BUILD_LOBBY_SERVER "Build the lobby server (requires libcurl)" OFF)
if(BUILD_LOBBY_SERVER)
    find_package(CURL REQUIRED)
    add_executable(lobby_server
        "${SRC_DIR}/lobby_main.cpp"
        "${SRC_DIR}/LobbyServer.cpp"
    )
    target_link_libraries(lobby_server PRIVATE
        SFML::Network
        SFML::System
        CURL::libcurl
    )
endif()
