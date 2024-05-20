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
 "include/net_message.h" 
 "include/net_tsqueue.h" 
 "include/net_connection.h" 
 "include/net_server.h" 
 "include/net_client.h" 
 "include/net_includes.h" 
 "include/net_common.h" 
 # "include/game_client.h" 
 # "include/game_server.h" 
 "include/game_structs.h"
 # "include/stl_container_check.h"
 "argparse/include/argparse.hpp"
 "olcPixelGameEngine/include/olcPixelGameEngine.h"
 "olcPixelGameEngine/include/olcPGEX_TransformedView.h")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/argparse/include")
include_directories("${PROJECT_SOURCE_DIR}/olcPixelGameEngine/include")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Add the executable
add_executable(ConsolePacmanProject ${SOURCES} ${HEADERS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASIO_STANDALONE")

if(LINUX OR MACOS)
    # find_package(Curses REQUIRED)
    # include_directories(${CURSES_INCLUDE_DIR})
    # target_link_libraries(ConsolePacmanProject ${CURSES_LIBRARIES})
    # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lncurses -DNCURSES_NOMACROS")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -lX11 -lGL -lpng -lstdc++fs")
    target_link_libraries(ConsolePacmanProject pthread X11 GL png stdc++fs)
elseif(WINDOWS)
    # set(CURSES_NEED_WIDE TRUE)
    # set(PDCurses_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/PDCurses/include")
    # set(PDCurses_LIBRARIES "${PROJECT_SOURCE_DIR}/PDCurses/lib/pdcurses.lib")
    # include_directories(${PDCurses_INCLUDE_DIRS})
    # target_link_libraries(ConsolePacmanProject ${PDCurses_LIBRARIES})
endif()

# Check and include ASIO
set(PATH_ASIO "${PROJECT_SOURCE_DIR}/asio-1.30.2/include")
target_include_directories(ConsolePacmanProject PRIVATE ${PATH_ASIO})