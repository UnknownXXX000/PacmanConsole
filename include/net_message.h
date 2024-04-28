#pragma once

#include "net_common.h"

constexpr static uint32_t MAGIC = 0xabcdfe01;

namespace net
{

	template<typename T>
	class connection;


	template<typename T>
	struct message_header
	{
		uint32_t magic = MAGIC;
		T ptype{};
		uint32_t datasize = 0;
	};

	template<typename T>
	struct message
	{
		message_header<T> header{};
		std::vector<uint8_t> data;

		// returns size of entire message packet in bytes
		[[nodiscard]] size_t size() const noexcept
		{
			return sizeof(message_header<T>) + data.size();
		}

		// Override for std::cout compatibility - produces friendly description of message
		friend std::ostream& operator<<(std::ostream& os, const message<T>& msg)
		{
			os << "PType: " << static_cast<int>(msg.header.ptype) << " Size: " << msg.header.datasize;
			return os;
		}

		// Convenience Operator overloads - These allow us to add and remove stuff from
		// the body vector as if it were a stack, so First in, Last Out. These are a 
		// template in itself, because we dont know what data type the user is pushing or 
		// popping, so lets allow them all. NOTE: It assumes the data type is fundamentally
		// Plain Old Data (POD). TLDR: Serialise & Deserialise into/from a vector

		// Pushes any POD-like data into the message buffer
		template<typename DataType>
		friend message<T>& operator << (message<T>& msg, const DataType& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to be pushed into vector");

			// Cache current size of vector, as this will be the point we insert the data
			const size_t i = msg.data.size();

			// Resize the vector by the size of the data being pushed
			msg.data.resize(i + sizeof(DataType));

			// Physically copy the data into the newly allocated vector space
			std::memcpy(msg.data.data() + i, &data, sizeof(DataType));

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}

		// Pulls any POD-like data form the message buffer
		template<typename DataType>
		friend message<T>& operator >> (message<T>& msg, DataType& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to be pulled from vector");

			// Cache the location towards the end of the vector where the pulled data starts
			const size_t i = msg.data.size() - sizeof(DataType);

			// Physically copy the data from the vector into the user variable
			std::memcpy(&data, msg.data.data() + i, sizeof(DataType));

			// Shrink the vector to remove read bytes, and reset end position
			msg.data.resize(i);

			// Recalculate the message size
			msg.header.size = msg.size();

			// Return the target message so it can be "chained"
			return msg;
		}
	};

	template <typename T>
	struct owned_message
	{
		std::shared_ptr<connection<T>> remote = nullptr;
		message<T> msg;

		// Again, a friendly string maker
		friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg_)
		{
			os << msg_.msg;
			return os;
		}
	};

	
}