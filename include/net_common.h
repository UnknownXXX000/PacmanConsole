#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <queue>
#include <optional>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <random>
#include <string>
#include <any>


#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

enum class PTypes : uint32_t
{
	CLIENT_SEND_KEY = 0x00,
	CLIENT_CONNECT = 0x01,
	CLIENT_READY = 0x02,
	SERVER_SEND_MAP = 0x10,
	SERVER_GAME_START = 0x20,
	SERVER_SEND_KEY_TO_OTHERS = 0xffffffff,
	// SERVER_PING = 0x50,
};

// typedef net::message<PTypes> GameMessage;

enum class PlayerMoves : uint8_t
{
	UP = 0,
	RIGHT = 1,
	DOWN = 2,
	LEFT = 3
};

enum class CellType : uint8_t
{
	EMPTY = 0x00,
	PLAYER = 0x22,
	FOOD = 0xaa,
	WALL = 0xff
};

struct player {
	uint32_t start_x;
	uint32_t start_y;
	uint32_t start_direction;
	uint32_t player_name_len;
	char player_name[256];
};



//template<typename T, typename std::enable_if_t<(std::is_same_v<T, uint32_t> || (std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, uint32_t>)), T>* = nullptr>
template<typename T, typename std::enable_if_t<std::is_standard_layout_v<T>, T>* = nullptr>
constexpr inline void ChangeEndian(T& value)
{
	if constexpr (sizeof(T) > 1u)
	{
		//auto val = value;
		std::array<uint8_t, sizeof(T)> valueBytes;
		std::memcpy(valueBytes.data(), &value, sizeof(T));
		std::reverse(valueBytes.begin(), valueBytes.end());
		std::memcpy(&value, valueBytes.data(), sizeof(T));
		//return val;
	}
	//return value;
}