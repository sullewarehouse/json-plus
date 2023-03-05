# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#

cmake_minimum_required (VERSION 3.8)
project ("json-plus")

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

add_executable(json-plus WIN32 "example.cpp" "json-plus.cpp" "json-plus.h")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT json-plus)
set_target_properties(json-plus PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
