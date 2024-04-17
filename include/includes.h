// PacmanConsole.h : включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта.

#pragma once

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
    /* UNIX-style OS. ------------------------------------------- */
#define IS_UNIX 1
#elif defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
// Windows
#define IS_WIN 1
#endif

#ifdef IS_WIN
#define _WIN32_WINNT 0x0A00
#endif

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <string>

// TODO: установите здесь ссылки на дополнительные заголовки, требующиеся для программы.

#ifdef IS_UNIX
    /* UNIX-style OS. ------------------------------------------- */
#include <ncurses.h>
#elif defined(IS_WIN)
// Windows
#include <curses.h>
#endif

#include <asio.hpp>