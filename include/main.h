// PacmanConsole.h : включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта.

#pragma once

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#define _WIN32_WINNT 0x0A00
#endif

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif


// TODO: установите здесь ссылки на дополнительные заголовки, требующиеся для программы.

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
    /* UNIX-style OS. ------------------------------------------- */
#include <ncurses.h>
#elif defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
// Windows
#include <curses.h>
#endif

#include <string>
#include <argparse.hpp>

#include "net_includes.h"