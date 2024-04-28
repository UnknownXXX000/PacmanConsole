#pragma once

#include "net_common.h"

#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"

namespace net
{
	template<typename T>
	class client_interface
	{
	public:
		client_interface(const client_interface& other) = delete;

		client_interface(client_interface&& other) noexcept = default;

		client_interface& operator=(const client_interface& other) = delete;

		client_interface& operator=(client_interface&& other) noexcept
		{
			m_context = std::move(other.m_context);
			thrContext = std::move(other.thrContext);
			m_connection = std::move(other.m_connection);
			m_qMessagesIn = std::move(other.m_qMessagesIn);
			return *this;
		}


		client_interface()
		{
		}

		virtual ~client_interface()
		{
			Disconnect();
		}

		// Connect to server with hostname/ip-address and port
		bool Connect(const std::string& host, const uint16_t port)
		{
			try
			{
				// Resolve hostname/ip-address into tangiable physical address
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

				// Create connection
				m_connection = std::make_unique<connection<T>>(connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessagesIn);

				// Tell the connection object to connect to server
				m_connection->ConnectToServer(endpoints);

				// Start Context Thread
				thrContext = std::thread([this]() { m_context.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "Client Exception: " << e.what() << "\n";
				return false;
			}
			return true;
		}

		// Disconnect from server
		void Disconnect()
		{
			// If connection exists, and it's connected then...
			if (IsConnected())
			{
				// ...disconnect from server gracefully
				m_connection->Disconnect();
			}

			// Either way, we're also done with the asio context...				
			m_context.stop();
			// ...and its thread
			if (thrContext.joinable())
				thrContext.join();

			// Destroy the connection object
			m_connection.release();
		}


		[[nodiscard]] bool IsConnected() const noexcept
		{
			if (m_connection)
				return m_connection->IsConnected();

			return false;
		}

		tsqueue<owned_message<T>>& Incoming()
		{
			return m_qMessagesIn;
		}

		// Send message to server
		void Send(const message<T>& msg)
		{
			if (IsConnected())
				m_connection->Send(msg);
		}

	protected:
		// asio context handles the data transfer...
		asio::io_context m_context;
		// ...but needs a thread of its own to execute its work commands
		std::thread thrContext;

		// The client has a single instance of a "connection" object, which handles data transfer
		std::unique_ptr<connection<T>> m_connection;

	private:
		// This is the thread safe queue of incoming messages from server
		tsqueue<owned_message<T>> m_qMessagesIn;
	};
}