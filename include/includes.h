#pragma once

#include <algorithm>
#include <mutex>
#include <string>
#include <unordered_set>

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
/* UNIX-style OS. ------------------------------------------- */
#define IS_UNIX 1
#elif defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
// Windows
#define IS_WIN 1
#endif

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif


// TODO: ���������� ����� ������ �� �������������� ���������, ����������� ��� ���������.

// #ifdef IS_UNIX
//     /* UNIX-style OS. ------------------------------------------- */
// #include <ncurses.h>
// #elif defined(IS_WIN)
// // Windows
// #include "../PDCurses/include/curses.h"
// #endif

#include <argparse.hpp>

#include "net_includes.h"