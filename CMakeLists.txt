cmake_minimum_required(VERSION 3.16)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

project (minesector VERSION 1.1.0)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf)
find_package(SDL2_image)
find_package(SDL2_mixer)

set (CMAKE_CXX_STANDARD 17)

if (APPLE AND CMAKE_BUILD_TYPE MATCHES Release)
    # Statically link on MacOS
    set(STATIC_LINK 1)
endif()

if (NOT FRONTEND)
    set(FRONTEND "NATIVE")
endif()

message(STATUS "FRONTEND=${FRONTEND}")
set(EXECUTABLE ${PROJECT_NAME})

if (FRONTEND STREQUAL "TEST")
    add_compile_definitions(FRONTEND_TEST)
    set(EXECUTABLE "testminesector")
elseif(FRONTEND STREQUAL "NATIVE")
    add_compile_definitions(FRONTEND_NATIVE)
else()
    message(FATAL_ERROR "Unkown FRONTEND=${FRONTEND} (should be TEST or NATIVE)")
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -std=c++17")
# FLAGS
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wno-attributes")
if (WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wl,-subsystem,windows")
endif()
if (WIN32 OR STATIC_LINK)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
    endif()
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
    src/frontend.cpp
)

set(WINDOWS_APP_ICON "${PROJECT_SOURCE_DIR}/icons/appicon.rc")

add_executable(${EXECUTABLE} MACOSX_BUNDLE ${SOURCES} ${WINDOWS_APP_ICON})

if (STATIC_LINK)
    target_link_libraries(${EXECUTABLE} SDL2::SDL2-static SDL2_image::SDL2_image-static SDL2_ttf::SDL2_ttf-static SDL2_mixer::SDL2_mixer-static)
elseif(WIN32)
    target_link_libraries(${EXECUTABLE} SDL2::SDL2 SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
else()
    target_link_libraries(${EXECUTABLE} SDL2::SDL2 SDL2_image SDL2_ttf SDL2_mixer)
endif()

if (UNIX)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()


if (APPLE)
    # MacOS bundle automatically points exe to Resource folder
    set(RUNTIME_BASE_PATH "")
else()
    set(RUNTIME_BASE_PATH "../share/${PROJECT_NAME}/")
endif()

message(STATUS "RUNTIME BASE PATH: ${RUNTIME_BASE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "PROJECT_VERSION: ${PROJECT_VERSION}")
message(STATUS "Cxx Compiler ID: ${CMAKE_CXX_COMPILER_ID}")

file(GLOB DATA_FONTS  "${PROJECT_SOURCE_DIR}/assets/fonts/*")
file(GLOB DATA_IMAGES "${PROJECT_SOURCE_DIR}/assets/images/*")
file(GLOB DATA_SOUNDS "${PROJECT_SOURCE_DIR}/assets/sounds/*")

#### MACOS BUNDLE BULLSHIT #####
set(MACOSX_BUNDLE_ICON_FILE appicon.icns) # Don't include the full path
target_sources(${EXECUTABLE} PRIVATE ${DATA_FONTS} ${DATA_IMAGES} ${DATA_SOUNDS} icons/appicon.icns)
set_source_files_properties(icons/appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${DATA_FONTS}  PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets/fonts)
set_source_files_properties(${DATA_IMAGES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets/images)
set_source_files_properties(${DATA_SOUNDS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets/sounds)

if (APPLE)       # MacOS app name
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "MineSector")
endif()

add_compile_definitions(RUNTIME_BASE_PATH="${RUNTIME_BASE_PATH}")

install(TARGETS ${EXECUTABLE}
        RUNTIME DESTINATION bin
        BUNDLE  DESTINATION .
)

if (NOT APPLE)
    install(DIRECTORY assets DESTINATION share/${PROJECT_NAME} USE_SOURCE_PERMISSIONS)
    if (UNIX)
        # Linux app icon
        install(FILES ${PROJECT_NAME}.desktop DESTINATION share/applications/)
        install(FILES icons/appicon.png RENAME ${PROJECT_NAME}.png DESTINATION share/icons/hicolor/128x128/apps/)
        install(FILES dev.rzia.minesector.appdata.xml DESTINATION share/metainfo)
    endif()
endif()

# Windows copy DLL's
if (WIN32)
    install(FILES "${SDL2_PREFIX}/bin/SDL2.dll" DESTINATION bin)
    install(DIRECTORY $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> DESTINATION bin)
endif()

set(CPACK_PACKAGE_NAME "MineSector")
set(CPACK_PACKAGE_FILE_NAME "minesector")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "minesector")
set(CPACK_PACKAGE_EXECUTABLES "minesector" "MineSector")
set(CPACK_CREATE_DESKTOP_LINKS "minesector")
set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/icons/appicon.png")
set(CPACK_WIX_PRODUCT_ICON "${PROJECT_SOURCE_DIR}/icons\\\\appicon.ico")
set(CPACK_PRODUCTBUILD_RESOURCES_DIR "assets")
set(CPACK_WIX_UPGRADE_GUID "66D7287B-D720-4920-9C0D-DB8CD2DC378A")
set(CPACK_PACKAGE_CONTACT "https://github.com/grassdne/minesector")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0 (>= 2.0.18), libsdl2-ttf-2.0-0, libsdl2-mixer-2.0-0, libsdl2-image-2.0-0")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif (WIN32)
    set(CPACK_GENERATOR "WIX")
elseif (UNIX)
    set(CPACK_GENERATOR "DEB" "RPM")
endif()

include(CPack)
