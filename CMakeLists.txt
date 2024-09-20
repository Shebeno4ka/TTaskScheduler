cmake_minimum_required(VERSION 3.13)

project(TTaskScheduler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -pedantic")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -pedantic")

set(SOURCES bin/main.cpp)
set(HEADERS lib)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
include_directories(${PROJECT_NAME} ${HEADERS})

add_subdirectory(tests)