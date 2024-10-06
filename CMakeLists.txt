cmake_minimum_required(VERSION 3.14)
project(Editor65 VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# This is the main part:
set(SOURCES main.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CURSES_LIBRARIES})

# Installation rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
