cmake_minimum_required(VERSION 3.16)

project (sdlminesweeper)

# awkward...    what am I supposed to do on windows :/
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
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-attributes")

set(SOURCES
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
    set(LIBS SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wl,-subsystem,windows")
endif()

target_link_libraries(${PROJECT_NAME} ${LIBS})

set(RUNTIME_BASE_PATH "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/")

include(CMakePrintHelpers)
cmake_print_variables(RUNTIME_BASE_PATH)

add_compile_definitions(RUNTIME_BASE_PATH="${RUNTIME_BASE_PATH}")

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}")
install(DIRECTORY assets DESTINATION share/${PROJECT_NAME})

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()
