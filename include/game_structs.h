#pragma once

enum class PTypes : uint32_t
{
	CLIENT_CONNECT				= 0x01,
	SERVER_SEND_MAP				= 0x10,
	CLIENT_READY				= 0x02,
	SERVER_GAME_START			= 0x20,
	CLIENT_SEND_KEY				= 0x00,
	SERVER_SEND_KEY_TO_OTHERS	= 0xffffffff,
	SERVER_PING					= 0x50,
	SERVER_ACCEPT				= 0x60,
};

enum class PlayerMoves : uint8_t
{
	UP		= 0,
	RIGHT	= 1,
	DOWN	= 2,
	LEFT	= 4
};

enum class CellType : uint8_t
{
	EMPTY	= 0x00,
	WALL	= 0xff,
	FOOD	= 0xaa,
	PLAYER	= 0x22
};

template<size_t WIDTH, size_t HEIGHT>
class GameField
{
private:
	CellType field[WIDTH][HEIGHT] = {0};
public:
	constexpr GameField() = default;
	~GameField() = default;

	GameField(const GameField&) = default;
	GameField(GameField&&) noexcept = default;

	[[nodiscard]] bool operator==(const GameField& other) const noexcept
	{
		return (std::memcmp(&field, &other.field, WIDTH * HEIGHT) == 0);
	}

	[[nodiscard]] bool operator!=(const GameField& other) const noexcept
	{
		return !(*this == other);
	}

	GameField& operator=(const GameField& other)
	{
		std::memcpy(&field, &other.field, WIDTH * HEIGHT);
		return *this;
	}

	GameField& operator=(GameField&& other) noexcept
	{
		std::memcpy(&field, &other.field, WIDTH * HEIGHT);
		return *this;
	}

	CellType& operator()(size_t x, size_t y)
	{
		static_assert(x < WIDTH && y < HEIGHT);
		return field[x][y];
	}

	const CellType& at(const size_t& x, const size_t& y) const
	{
		static_assert(x < WIDTH && y < HEIGHT);
		return field[x][y];
	}

	constexpr void fill(CellType value)
	{
		std::memset(&field[0], value, WIDTH * HEIGHT);
	}

	[[nodiscard]] constexpr size_t GetWidth() const noexcept
	{
		return WIDTH;
	}

	[[nodiscard]] constexpr size_t GetHeight() const noexcept
	{
		return HEIGHT;
	}

	void InvertVertical()
	{
		static_assert(HEIGHT % 2 == 0);

		for (size_t i = 0; i < WIDTH; i++)
			for (size_t j = 0; j < HEIGHT / 2; j++)
				std::swap(field[i][j], field[i][HEIGHT - j]);
	}

	void InvertHorizontal()
	{
		static_assert(WIDTH % 2 == 0);

		for (size_t i = 0; i < WIDTH / 2; i++)
			for (size_t j = 0; j < HEIGHT; j++)
				std::swap(field[i][j], field[WIDTH - i][j]);
	}
};