#pragma once

#include "net_common.h"
// #include "stl_container_check.h"

constexpr static uint32_t MAGIC = 0xabcdfe01;
constexpr static uint32_t MAGIC_LITTLE = 0x01fecdab;

//template<typename T, typename std::enable_if_t<std::is_standard_layout_v<T>, T>* = nullptr>
//constexpr inline void ChangeEndian(T& value);

struct player;

/*
template<typename T, typename std::enable_if_t<std::is_standard_layout_v<T> && !(std::is_unsigned_v<T> || std::is_enum_v<T>), T>* = nullptr>
void foo(const T& data)
{
	// static_assert(std::is_standard_layout<T>::value, "Data too complex");
	std::cout << typeid(data).name() << " main called\n";
}

template<typename T, typename std::enable_if_t<std::is_unsigned_v<T>, T>* = nullptr>
void foo(const T& data)
{
	std::cout << typeid(data).name() << "unsigned called\n";
}

template<typename T, std::enable_if_t<std::is_enum_v<T> && std::is_unsigned_v<std::underlying_type_t<T>>,T>* = nullptr>
void foo(const T& data)
{
	std::cout << typeid(std::underlying_type_t<T>).name() << " " << typeid(T).name() << " called\n";
}
*/

namespace net
{
	template<typename T>
	class connection;


	template<typename T>
	struct message_header
	{
		uint32_t magic = 0xabcdfe01;
		T id{};
		uint32_t size = 0;
	};

	// Message Body contains a header and a std::vector, containing raw bytes
	// of infomation. This way the message can be variable length, but the size
	// in the header must be updated.
	template <typename T>
	struct message
	{
		// Header & Body vector
		message_header<T> header{};
		std::vector<uint8_t> body;

		// returns size of entire message packet in bytes
		size_t size() const
		{
			return body.size();
		}

		// Override for std::cout compatibility - produces friendly description of message
		friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
		{
			os << "ID:" << static_cast<int>(msg.header.id) << " Size:" << msg.header.size;
			return os;
		}

		// Convenience Operator overloads - These allow us to add and remove stuff from
		// the body vector as if it were a stack, so First in, Last Out. These are a 
		// template in itself, because we dont know what data type the user is pushing or 
		// popping, so lets allow them all. NOTE: It assumes the data type is fundamentally
		// Plain Old Data (POD). TLDR: Serialise & Deserialise into/from a vector

		// Pushes any POD-like data into the message buffer
		template<typename DataType, typename std::enable_if_t<std::is_standard_layout_v<DataType> && !(std::is_unsigned_v<DataType> && sizeof(DataType) == 4u) && !std::is_same_v<DataType, player>, DataType>* = nullptr>
		friend message<T>& operator << (message<T>& msg, const DataType& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			//static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to be pushed into vector");

			// Cache current size of vector, as this will be the point we insert the data
			size_t i = msg.body.size();

			// Resize the vector by the size of the data being pushed
			msg.body.resize(msg.body.size() + sizeof(DataType));

			// Physically copy the data into the newly allocated vector space
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

		// Pulls any POD-like data form the message buffer
		template<typename DataType, typename std::enable_if_t<std::is_standard_layout_v<DataType> && !(std::is_unsigned_v<DataType> && sizeof(DataType) == 4u) && !std::is_same_v<DataType, player>, DataType>* = nullptr>
		friend message<T>& operator >> (message<T>& msg, DataType& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			// static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to be pulled from vector");

			// Cache the location towards the end of the vector where the pulled data starts
			size_t i = msg.body.size() - sizeof(DataType);

			// Physically copy the data from the vector into the user variable
			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

			// Shrink the vector to remove read bytes, and reset end position
			msg.body.resize(i);

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

		// For unsigned 4 byte values
		template<typename DataType, typename std::enable_if_t<(std::is_unsigned_v<DataType> && sizeof(DataType) == 4u), DataType>* = nullptr>
		friend message<T>& operator << (message<T>& msg, const DataType& data)
		{
			auto dataCopy = data;
			ChangeEndian(dataCopy);
			// Cache current size of vector, as this will be the point we insert the data
			size_t i = msg.body.size();

			// Resize the vector by the size of the data being pushed
			msg.body.resize(msg.body.size() + sizeof(DataType));

			// Physically copy the data into the newly allocated vector space
			std::memcpy(msg.body.data() + i, &dataCopy, sizeof(DataType));

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

		// For unsigned 4 byte values
		template<typename DataType, typename std::enable_if_t<(std::is_unsigned_v<DataType> && sizeof(DataType) == 4u), DataType>* = nullptr>
		friend message<T>& operator >> (message<T>& msg, DataType& data)
		{
			// Cache the location towards the end of the vector where the pulled data starts
			size_t i = msg.body.size() - sizeof(DataType);

			// Physically copy the data from the vector into the user variable
			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

			ChangeEndian(data);

			// Shrink the vector to remove read bytes, and reset end position
			msg.body.resize(i);

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

		// For player struct
		friend message<T>& operator << (message<T>& msg, const player& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			//static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to be pushed into vector");

			auto playerCopy = data;

			ChangeEndian(playerCopy.start_x);
			ChangeEndian(playerCopy.start_y);
			ChangeEndian(playerCopy.start_direction);
			ChangeEndian(playerCopy.player_name_len);

			// Cache current size of vector, as this will be the point we insert the data
			size_t i = msg.body.size();

			// Resize the vector by the size of the data being pushed
			msg.body.resize(msg.body.size() + sizeof(player));

			// Physically copy the data into the newly allocated vector space
			std::memcpy(msg.body.data() + i, &playerCopy, sizeof(player));

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

		// For player struct
		friend message<T>& operator >> (message<T>& msg, player& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			// static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to be pulled from vector");

			// Cache the location towards the end of the vector where the pulled data starts
			size_t i = msg.body.size() - sizeof(player);

			// Physically copy the data from the vector into the user variable
			std::memcpy(&data, msg.body.data() + i, sizeof(player));

			ChangeEndian(data.start_x);
			ChangeEndian(data.start_y);
			ChangeEndian(data.start_direction);
			ChangeEndian(data.player_name_len);

			// Shrink the vector to remove read bytes, and reset end position
			msg.body.resize(i);

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

		// Reverse byte order of header
		void ReverseHeader()
		{
			ChangeEndian(header.magic);
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4u) ChangeEndian(header.id);
			ChangeEndian(header.size);
		}
	};


	// An "owned" message is identical to a regular message, but it is associated with
	// a connection. On a server, the owner would be the client that sent the message, 
	// on a client the owner would be the server.

	// Forward declare the connection
	template <typename T>
	class connection;

	template <typename T>
	struct owned_message
	{
		std::shared_ptr<connection<T>> remote = nullptr;
		message<T> msg;

		// Again, a friendly string maker
		friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};

}