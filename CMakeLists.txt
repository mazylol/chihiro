cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(chihiro)

# env file
set(ENV_FILE_SOURCE ${CMAKE_SOURCE_DIR}/.env)
    set(ENV_FILE_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/.env)
    configure_file(${ENV_FILE_SOURCE} ${ENV_FILE_DESTINATION} COPYONLY)

# Enable compile commands export for easier debugging
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Option to use ccache for faster builds
option(USE_CCACHE "Use ccache for compilation" ON)
if (USE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if (CCACHE_PROGRAM)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    endif ()
endif ()

include(cmake/CPM.cmake)

CPMAddPackage("gh:brainboxdotcc/DPP@10.0.28")
CPMAddPackage("gh:mazylol/dotenvloader@1.0.1")
CPMAddPackage("gh:libcpr/cpr#1.11.1")
CPMAddPackage("gh:fmtlib/fmt#11.0.2")
CPMAddPackage("gh:nlohmann/json@3.11.3")

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE dotenvloader dpp cpr::cpr fmt::fmt nlohmann_json::nlohmann_json)
