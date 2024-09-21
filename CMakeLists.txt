cmake_minimum_required(VERSION 3.13)

project(TTaskScheduler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES bin/main.cpp)
set(HEADERS lib)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
# Условные флаги компиляции
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

include_directories(${PROJECT_NAME} ${HEADERS})

add_subdirectory(tests)