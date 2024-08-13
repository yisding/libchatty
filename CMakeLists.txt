cmake_minimum_required(VERSION 3.7)

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

add_library(libchatty chatty.c chatty.h cJSON.c cJSON.h)
set_target_properties(libchatty PROPERTIES OUTPUT_NAME chatty)
target_link_libraries(libchatty PUBLIC curl)

add_executable(chatty main.c)
target_link_libraries(chatty PUBLIC libchatty)
