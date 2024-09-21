cmake_minimum_required(VERSION 3.13)

project(TTaskScheduler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

if(msvc)
  set(COMPILE_COMMANDS "/W4 /WX /permissive-")
else()
  set(COMPILE_COMMANDS "-Wall -Wextra -pedantic")
endif()

set(CMAKE_CXX_FLAGS_DEBUG ${COMPILE_COMMANDS})
set(CMAKE_CXX_FLAGS_RELEASE ${COMPILE_COMMANDS})

set(SOURCES bin/main.cpp)
set(HEADERS lib)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
include_directories(${PROJECT_NAME} ${HEADERS})

add_subdirectory(tests)