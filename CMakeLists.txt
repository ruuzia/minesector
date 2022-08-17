cmake_minimum_required(VERSION 3.16)

project (sdlminesweeper)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "C:\\mingw_dev\\lib")

find_package(SDL2 REQUIRED)

# These packages arent on UNIX
if (WIN32)
    find_package(SDL2_ttf)
    find_package(SDL2_image)
    find_package(SDL2_mixer)
endif()

set (CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE, Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -std=c++17")
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-attributes")

set( SOURCES
    src/main.cpp
    src/texture.cpp
    src/game.cpp
    src/button.cpp
    src/anim.cpp
    src/text.cpp
    src/color.cpp
    src/font.cpp
    src/tile.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})


if (UNIX)
    include_directories(${SDL2_INCLUDE_DIRS})
    set(LIBS ${SDL2_LIBRARIES} SDL2_image SDL2_ttf SDL2_mixer)

elseif (WIN32)
    set (LIBS SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wl,-subsystem,windows")
endif()


target_link_libraries(${PROJECT_NAME} ${LIBS})
