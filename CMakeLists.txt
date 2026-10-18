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

# ---------------------------------------------------------------------------
# Windows hand-off: make build/Release a folder that can be zipped and played
# without installing anything but Tailscale.
#   cmake --build build --config Release --target package_zip
# ---------------------------------------------------------------------------
if(WIN32)
    set(LOBBY_HOST "100.112.152.113" CACHE STRING "Lobby address written into play.bat")

    # SFML DLLs (incl. openal32.dll) from <SFML>/bin
    get_filename_component(SFML_ROOT "${SFML_DIR}/../../.." ABSOLUTE)
    file(GLOB SFML_DLLS "${SFML_ROOT}/bin/*.dll")
    foreach(dll IN LISTS SFML_DLLS)
        add_custom_command(TARGET the_last_paladin POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll}" "$<TARGET_FILE_DIR:the_last_paladin>"
            VERBATIM)
    endforeach()

    # The prebuilt SFML DLLs use the dynamic MSVC runtime, so ship its DLLs
    # next to the exe (app-local deployment) instead of requiring the redistributable.
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP OFF)
    include(InstallRequiredSystemLibraries)
    foreach(dll IN LISTS CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        add_custom_command(TARGET the_last_paladin POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll}" "$<TARGET_FILE_DIR:the_last_paladin>"
            VERBATIM)
    endforeach()

    file(GENERATE OUTPUT "$<TARGET_FILE_DIR:the_last_paladin>/play.bat"
        CONTENT "@echo off\r\nset LOBBY_HOST=${LOBBY_HOST}\r\nthe_last_paladin.exe\r\n")

    add_custom_target(package_zip
        COMMAND ${CMAKE_COMMAND} -E tar cf "${CMAKE_BINARY_DIR}/TheLastPaladin.zip" --format=zip .
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:the_last_paladin>"
        DEPENDS the_last_paladin
        COMMENT "Creating TheLastPaladin.zip")
endif()

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
