# CMakeList.txt: проект CMake для PacmanConsole; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required(VERSION 3.10)
project(ConsolePacmanProject)

# Check the operating system
if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
elseif(APPLE)
    set(MACOS TRUE)
elseif(WIN32)
    set(WINDOWS TRUE)
endif()

# Add source files
set(SOURCES
    "src/main.cpp"
    #src/SnakeGame.cpp
)

# Add header files
set (HEADERS
    "include/includes.h"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Add the executable
add_executable(ConsolePacmanProject ${SOURCES} ${HEADERS})

# Check and include NCurses or PDCurses
if(LINUX OR MACOS)
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    target_link_libraries(ConsolePacmanProject ${CURSES_LIBRARIES})
elseif(WINDOWS)
    set(CURSES_NEED_WIDE TRUE)
    set(PDCurses_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
    set(PDCurses_LIBRARIES "${PROJECT_SOURCE_DIR}/lib/pdcurses.lib")
    include_directories(${PDCurses_INCLUDE_DIRS})
    target_link_libraries(ConsolePacmanProject ${PDCurses_LIBRARIES})
endif()

# Check and include ASIO
set(PATH_ASIO "${PROJECT_SOURCE_DIR}/asio-1.30.2/include")
target_include_directories(ConsolePacmanProject PRIVATE ${PATH_ASIO})