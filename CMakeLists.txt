cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.29)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.29)
endif()

project(
  libchatty
  VERSION 0.1
  DESCRIPTION "LLM wrapper for ricers"
  LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# vcpkg toolchain detection
if(NOT CMAKE_TOOLCHAIN_FILE AND NOT DEFINED ENV{VCPKG_ROOT})
    # Try to find vcpkg installation
    find_program(VCPKG_EXECUTABLE vcpkg)
    if(VCPKG_EXECUTABLE)
        get_filename_component(VCPKG_ROOT ${VCPKG_EXECUTABLE} DIRECTORY)
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        message(STATUS "Found vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    else()
        message(WARNING "vcpkg not found in PATH. Consider installing vcpkg or setting CMAKE_TOOLCHAIN_FILE manually")
    endif()
elseif(NOT CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain from VCPKG_ROOT: ${CMAKE_TOOLCHAIN_FILE}")
endif()

find_package(curl CONFIG REQUIRED)

add_library(libchatty chatty.c chatty.h cJSON.c cJSON.h)
set_target_properties(libchatty PROPERTIES 
    OUTPUT_NAME chatty
    POSITION_INDEPENDENT_CODE ON
)

# Set include directories
target_include_directories(libchatty PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Add compiler warnings
target_compile_options(libchatty PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<C_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(libchatty PUBLIC CURL::libcurl)

add_executable(chatty main.c)
target_link_libraries(chatty PUBLIC libchatty)
