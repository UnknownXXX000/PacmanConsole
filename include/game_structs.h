#pragma once

#include <cassert>
#include "net_common.h"
// #include <mutex>

constexpr static unsigned int MAX_PLAYER_NAME_LEN = 256;
constexpr static unsigned int MAP_PART_SIZE = 20 * 15;

//#ifndef OLC_PGE_APPLICATION
//#define OLC_PGE_APPLICATION
//#endif
//
//#include <olcPixelGameEngine.h>
//
//#ifndef OLC_PGEX_TRANSFORMEDVIEW
//#define OLC_PGEX_TRANSFORMEDVIEW
//#endif
//
//#include <olcPGEX_TransformedView.h>

inline uint8_t convert(const CellType& c)
{
	switch (c)
	{
	case CellType::EMPTY:
		return 0;
	case CellType::PLAYER:
		return 0x22;
	case CellType::FOOD:
		return 0xaa;
	case CellType::WALL:
		return 0xff;
	}
}

template<size_t WIDTH, size_t HEIGHT>
class GameField
{
private:
	union {
		CellType field[WIDTH][HEIGHT];
		uint8_t	 bytes[HEIGHT * WIDTH];

		// Correct: field[HEIGHT][WIDTH];
	};
	// std::mutex Mutex;
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

		for (size_t j = 0; j < HEIGHT; j++)
			for (size_t i = 0; i < WIDTH / 2; i++)
			{
				const auto temp = field[i][j];
				// std::swap(field[i][j], field[WIDTH - i][j]);
				field[i][j] = field[WIDTH - i - 1][j];
				field[WIDTH - i - 1][j] = temp;
			}
	}

	template<size_t OTHER_WIDTH, size_t OTHER_HEIGHT>
	void InsertToAnother(GameField<OTHER_WIDTH, OTHER_HEIGHT>& other, const size_t offset_x, const size_t offset_y)
	{
		assert(offset_x + WIDTH <= OTHER_WIDTH && offset_y + HEIGHT <= OTHER_HEIGHT);

		for (size_t j = 0; j < HEIGHT; j++)
			for (size_t i = 0; i < WIDTH; i++)
				other(i + offset_x, j + offset_y) = field[i][j];
	}

	public:
	friend std::ostream& operator<<(std::ostream& os, const GameField& f)
	{
		for (size_t j = 0; j < f.GetHeight(); j++)
		{
			for (size_t i = 0; i < f.GetWidth(); i++)
			{
				char c = '\0';
				switch (f.at(i, j))
				{
					case CellType::EMPTY:
						c = ' ';
						break;
					case CellType::WALL:
						c = '*';
						break;
					case CellType::FOOD:
						c = '0';
						break;
					case CellType::PLAYER:
						c = '+';
						break;
					default:
						break;
				}
				os << c;
			}
			os << '\n';
		}
		return os;
	}

	void GetStartPosition(uint32_t& x, uint32_t& y) const
	{
		bool bDone = false;
		// // std::scoped_lock lock(Mutex);
		for (size_t i = 0; i < WIDTH; i++)
		{
			for (size_t j = 0; j < HEIGHT; j++)
			{
				if (field[i][j] == CellType::EMPTY || field[i][j] == CellType::FOOD)
				{
					x = i;
					y = j;
					bDone = true;
					break;
				}
			}
			if (bDone) break;
		}
	}

	public:
	std::array<uint8_t, WIDTH * HEIGHT> ToBytes() const noexcept
	{
		// std::scoped_lock lock(Mutex);
		std::array<uint8_t, WIDTH * HEIGHT> byteArray;
		std::memcpy(byteArray.data(), &bytes[0], WIDTH * HEIGHT);
		return byteArray;
	}

	void FromBytes(const std::array<uint8_t, WIDTH * HEIGHT>& byteArray)
	{
		// std::scoped_lock lock(Mutex);
		std::memcpy(&bytes[0], byteArray.data(), WIDTH * HEIGHT);
	}

	void WriteToCArray(uint8_t(&array)[WIDTH * HEIGHT])
	{
		// std::scoped_lock lock(Mutex);
		std::memcpy(&array[0], &bytes[0], WIDTH * HEIGHT);
	}

	void ReadFromCArray(uint8_t(&array)[WIDTH * HEIGHT])
	{
		// std::scoped_lock lock(Mutex);
		std::memcpy(&bytes[0], &array[0], WIDTH * HEIGHT);
	}

	void WriteToCArrayFixed(uint8_t(&array)[WIDTH * HEIGHT])
	{
		size_t index = 0;
		// CellType field[WIDTH][HEIGHT];

		for (size_t i = 0; i < HEIGHT; i++)
		{
			for (size_t j = 0; j < WIDTH; j++)
			{
				array[index] = static_cast<uint8_t>(field[j][i]);
				index++;
			}
		}
	}

	void ReadFromCArrayFixed(uint8_t(&array)[WIDTH * HEIGHT])
	{
		size_t index = 0;
		// CellType field[WIDTH][HEIGHT];

		for (size_t i = 0; i < HEIGHT; i++)
		{
			for (size_t j = 0; j < WIDTH; j++)
			{
				field[j][i] = static_cast<CellType>(array[index]);
				index++;
			}
		}
	}

	uint8_t* data() const noexcept
	{
		// std::scoped_lock lock(Mutex);
		return &bytes[0];
	}

	void GenerateMap()
	{
		MazeGenerator(*this);
	}

	template<size_t OTHER_WIDTH, size_t OTHER_HEIGHT>
	void GenerateSymmetricMap(GameField<OTHER_WIDTH, OTHER_HEIGHT>& other)
	{
		assert(WIDTH * 2 == OTHER_WIDTH && HEIGHT * 2 == OTHER_HEIGHT);

		this->InsertToAnother(other, 0, 0);
		this->InvertVertical();
		this->InsertToAnother(other, 0, HEIGHT);
		this->InvertHorizontal();
		this->InsertToAnother(other, WIDTH, HEIGHT);
		this->InvertVertical();
		this->InsertToAnother(other, WIDTH, 0);
	}
};

template<size_t WIDTH, size_t HEIGHT>
class MazeGenerator
{
	friend GameField<WIDTH, HEIGHT>;

private:

	GameField<WIDTH, HEIGHT>& field;

	const int NUM_OBSTACLES = 5;

	// define the tetris shapes
	const int TETRIS_SHAPES[7][4][2] = {
	  {{0,0}, {1,0}, {0,1}, {1,1}}, // square
	  {{0,0}, {1,0}, {2,0}, {3,0}}, // horizontal line
	  {{0,0}, {0,1}, {0,2}, {0,3}}, // vertical line
	  {{0,0}, {1,0}, {1,1}, {2,1}}, // L shape
	  {{0,0}, {1,0}, {1,1}, {1,2}}, // inverted L shape
	  {{0,0}, {1,0}, {1,1}, {2,0}}, // T shape
	  {{0,0}, {1,0}, {1,1}, {2,1}}  // S shape
	};
	
public:
	MazeGenerator() = delete;
	~MazeGenerator() = default;
	
	MazeGenerator(const MazeGenerator&) = delete;
	MazeGenerator(MazeGenerator&&) noexcept = delete;
	MazeGenerator& operator=(const MazeGenerator&) = delete;
	MazeGenerator& operator=(MazeGenerator&&) noexcept = delete;

private:
	MazeGenerator(GameField<WIDTH, HEIGHT>& f) : field(f)
	{
		srand((unsigned)time(NULL));
		generate_obstacle_field();
	}

private:

	// check if a cell is inside the field
	inline bool is_inside(int x, int y) {
		return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
	}

	// check if a cell is an obstacle
	inline bool is_obstacle(int x, int y) {
		return field(x, y) == CellType::WALL;
	}

	// check if a cell is empty
	inline bool is_empty(int x, int y) {
		return field(x, y) == CellType::FOOD || field(x, y) == CellType::EMPTY;
	}

	int non_obstacle_neighbors(int x, int y) {
		int count = 0;
		const bool cell_inside = is_inside(x, y);
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx == 0 && dy == 0) continue;
				if ((is_inside(x + dx, y + dy) && is_empty(x + dx, y + dy)) || (!is_inside(x + dx, y + dy) && cell_inside)) {
					count++;
				}
			}
		}
		return count;
	}

	// check if a cell has at least 2 non-obstacle neighbors
	bool has_non_obstacle_neighbors(int x, int y) {
		int count = 0;
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx == 0 && dy == 0) continue;
				if (is_inside(x + dx, y + dy) && is_empty(x + dx, y + dy)) {
					count++;
				}
			}
		}
		return count >= 2;
	}

	// check if a tetris shape can be placed at a given position without intersecting with any obstacle
	bool can_place_tetris_shape(int x, int y, int shape_index) {
		for (int i = 0; i < 4; i++) {
			int dx = TETRIS_SHAPES[shape_index][i][0];
			int dy = TETRIS_SHAPES[shape_index][i][1];
			if (!is_inside(x + dx, y + dy) || is_obstacle(x + dx, y + dy)) {
				return false;
			}
			if (non_obstacle_neighbors(x + dx, y + dy) < 8) return false;
		}
		return true;
	}

	// place a tetris shape at a given position
	void place_tetris_shape(int x, int y, int shape_index) {
		for (int i = 0; i < 4; i++) {
			int dx = TETRIS_SHAPES[shape_index][i][0];
			int dy = TETRIS_SHAPES[shape_index][i][1];
			field(x + dx, y + dy) = CellType::WALL;
		}
	}

	// generate a random tetris shape
	int random_tetris_shape() {
		return rand() % 7;
	}

	// generate a random position for a tetris shape
	void random_position(int& x, int& y) {
		x = rand() % WIDTH;
		y = rand() % HEIGHT;
	}

	// check if two tetris shapes are colliding
	bool are_colliding(int x1, int y1, int shape_index1, int x2, int y2, int shape_index2) {
		for (int i = 0; i < 4; i++) {
			int dx1 = TETRIS_SHAPES[shape_index1][i][0];
			int dy1 = TETRIS_SHAPES[shape_index1][i][1];
			for (int j = 0; j < 4; j++) {
				int dx2 = TETRIS_SHAPES[shape_index2][j][0];
				int dy2 = TETRIS_SHAPES[shape_index2][j][1];
				if (x1 + dx1 == x2 + dx2 && y1 + dy1 == y2 + dy2) {
					return true;
				}
			}
		}
		return false;
	}


	void generate_obstacle_field() {
		for (int i = 0; i < WIDTH; i++) {
			for (int j = 0; j < HEIGHT; j++) {
				field(i, j) = CellType::FOOD;
			}
		}

		for (int i = 0; i < NUM_OBSTACLES; i++) {
			int shape_index1 = random_tetris_shape();
			int x1, y1;
			do {
				random_position(x1, y1);
			} while (!can_place_tetris_shape(x1, y1, shape_index1));
			place_tetris_shape(x1, y1, shape_index1);

			// check for collisions with other obstacles
			for (int j = 0; j < i; j++) {
				int shape_index2 = random_tetris_shape();
				int x2, y2;
				do {
					random_position(x2, y2);
				} while (!can_place_tetris_shape(x2, y2, shape_index2) || are_colliding(x1, y1, shape_index1, x2, y2, shape_index2));
				place_tetris_shape(x2, y2, shape_index2);
			}
		}

		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				if (is_empty(x, y) && !has_non_obstacle_neighbors(x, y)) {
					field(x, y) = CellType::WALL;
				}
			}
		}
	}
};

typedef GameField<20, 15> FieldPart;
typedef GameField<40, 30> FieldMap;

struct CppPlayer
{
	uint32_t start_x;
	uint32_t start_y;
	PlayerMoves start_direction;
	std::string name;

	player ToCPlayer() const
	{
		player p = { 0 };
		p.start_x = start_x;
		p.start_y = start_y;
		p.start_direction = static_cast<uint32_t>(start_direction);
		p.player_name_len = name.size();
		std::copy(name.begin(), name.end(), p.player_name);
		p.player_name[name.size()] = '\0';

		return p;
	}

	void FromCPlayer(const player& p)
	{
		start_x = p.start_x;
		start_y = p.start_y;
		start_direction = static_cast<PlayerMoves>(p.start_direction);
		name = std::string(p.player_name, p.player_name_len);
	}

	friend std::ostream& operator<<(std::ostream& os, const CppPlayer& cp)
	{
		os << "{ start_x = " << cp.start_x << ", start_y = " << cp.start_y << ", direction = " << static_cast<uint32_t>(cp.start_direction) << ", name = " << cp.name << " }";
		return os;
	}
};