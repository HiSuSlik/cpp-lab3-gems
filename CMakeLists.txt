cmake_minimum_required(VERSION 3.22)
project(GemsLabSFML LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SFML)

add_executable(GemsLabSFML WIN32
    src/main.cpp
    src/Board.cpp
    src/Game.cpp
    include/Config.h
    include/Types.h
    include/Board.h
    include/Game.h
)

target_include_directories(GemsLabSFML PRIVATE include)
target_link_libraries(GemsLabSFML PRIVATE sfml-graphics sfml-window sfml-system sfml-main)

if (WIN32)
    add_custom_command(TARGET GemsLabSFML POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:GemsLabSFML> $<TARGET_FILE_DIR:GemsLabSFML>
        COMMAND_EXPAND_LISTS
    )
endif()
