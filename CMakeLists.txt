cmake_minimum_required(VERSION 3.16)

project (sdlminesweeper VERSION 1.1.0)

find_package(SDL2 REQUIRED)

# There aren't packages for these on Unix
if (WIN32)
    find_package(SDL2_ttf)
    find_package(SDL2_image)
    find_package(SDL2_mixer)
endif()

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-attributes")
if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

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

set(WINDOWS_APP_ICON "${PROJECT_SOURCE_DIR}/icons/appicon.rc")

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${WINDOWS_APP_ICON})

if (UNIX)
    include_directories(${SDL2_INCLUDE_DIRS})
    set(LIBS ${SDL2_LIBRARIES} SDL2_image SDL2_ttf SDL2_mixer)
elseif (WIN32)
    set(LIBS SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wl,-subsystem,windows")
endif()

target_link_libraries(${PROJECT_NAME} ${LIBS})

if (APPLE)
    # MacOS Bundle
    set(RUNTIME_BASE_PATH "")
else()
    set(RUNTIME_BASE_PATH "../share/${PROJECT_NAME}/")
endif()

message(STATUS "RUNTIME BASE PATH: ${RUNTIME_BASE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "PROJECT_VERSION: ${PROJECT_VERSION}")
message(STATUS "PROJECT_VERSION_MAJOR: ${PROJECT_VERSION_MAJOR}")
message(STATUS "PROJECT_VERSION_MINOR: ${PROJECT_VERSION_MINOR}")

file(GLOB DATA_FONTS  "${PROJECT_SOURCE_DIR}/assets/fonts/*")
file(GLOB DATA_IMAGES "${PROJECT_SOURCE_DIR}/assets/images/*")
file(GLOB DATA_SOUNDS "${PROJECT_SOURCE_DIR}/assets/sounds/*")

set(MACOSX_BUNDLE_ICON_FILE appicon.icns) # Don't include the full path
target_sources(${PROJECT_NAME} PRIVATE ${DATA_FONTS} ${DATA_IMAGES} ${DATA_SOUNDS} icons/appicon.icns)
set_source_files_properties(icons/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${DATA_FONTS}  PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets/fonts)
set_source_files_properties(${DATA_IMAGES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets/images)
set_source_files_properties(${DATA_SOUNDS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets/sounds)

if (APPLE)       # MacOS app name
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Minesweeper")
endif()

add_compile_definitions(RUNTIME_BASE_PATH="${RUNTIME_BASE_PATH}")

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        BUNDLE  DESTINATION .
)

if (NOT APPLE)
    install(DIRECTORY assets DESTINATION share/${PROJECT_NAME} USE_SOURCE_PERMISSIONS)
    if (UNIX)
        # Linux app icon
        install(FILES ${PROJECT_NAME}.desktop DESTINATION share/applications)
        install(FILES icons/appicon.png RENAME ${PROJECT_NAME}.png DESTINATION share/pixmaps)
    endif()
endif()

if (WIN32 AND CMAKE_BUILD_TYPE MATCHES Release)
    install(DIRECTORY $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> DESTINATION bin)
    install(FILES "${SDL2_PREFIX}bin/SDL2.dll" DESTINATION bin)
endif()

set(CPACK_PACKAGE_NAME "Minesweeper")
set(CPACK_PACKAGE_FILE_NAME "sdlminesweeper-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "sdlminesweeper")
# set(CPACK_PACKAGE_VENDOR "grassdne")
set(CPACK_PACKAGE_EXECUTABLES "sdlminesweeper" "Minesweeper")
set(CPACK_CREATE_DESKTOP_LINKS "sdlminesweeper")
set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/icons/appicon.png")
set(CPACK_WIX_PRODUCT_ICON "${PROJECT_SOURCE_DIR}/icons\\\\appicon.ico")
set(CPACK_PRODUCTBUILD_RESOURCES_DIR "assets")
set(CPACK_WIX_UPGRADE_GUID "66D7287B-D720-4920-9C0D-DB8CD2DC378A")

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif (WIN32)
    set(CPACK_GENERATOR "WIX")
elseif (UNIX)
    set(CPACK_GENERATOR "DEB" "RPM")
endif()

include(CPack)
