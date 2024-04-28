#pragma once

#include "net_tsqueue.h"
#include "net_message.h"

namespace net
{
	template<typename T>
	class server_interface;

	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>>
	{
	public:
		enum class owner : bool
		{
			server,
			client
		};

		connection(owner parent,
			asio::io_context& asioContext,
			asio::ip::tcp::socket socket,
			tsqueue<owned_message<T>>& qIn)
				:	m_nOwnerType(parent),
					m_asioContext(asioContext),
					m_socket(std::move(socket)),
					m_qMessagesIn(qIn)
		{
			if (m_nOwnerType == owner::server)
			{
				m_nHandshakeOut = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());

				m_nHandshakeCheck = scramble(m_nHandshakeOut);
			}
			else
			{
				m_nHandshakeIn = 0;
				m_nHandshakeOut = 0;
			}

		}

		virtual ~connection()
		{

		}

		void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
		{
			// Only clients can connect to servers
			if (m_nOwnerType == owner::client)
			{
				// Request asio attempts to connect to an endpoint
				asio::async_connect(m_socket, endpoints,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
						{
							// ReadHeader();

							ReadValidation();
						}
					});
			}
		}
		
		void Disconnect()
		{
			if (IsConnected())
				asio::post(m_asioContext, [this]() { m_socket.close(); });
		}

		[[nodiscard]] bool IsConnected() const noexcept
		{
			return m_socket.is_open();
		}

		[[nodiscard]] uint32_t GetID() const noexcept
		{
			return id;
		}

		void Send(const message<T>& msg)
		{
			asio::post(m_asioContext,
				[this, msg]()
				{
					const bool bWritingMessage = !m_qMessagesOut.empty();
					m_qMessagesOut.push_back(msg);
					if (!bWritingMessage)
						WriteHeader();
				}
			);
		}

		void ConnectToClient(net::server_interface<T>* server, uint32_t uid = 0)
		{
			if (m_nOwnerType == owner::server)
			{
				if (m_socket.is_open())
				{
					id = uid;

					// ReadHeader();

					WriteValidation();

					ReadValidation(server);
				}
			}
		}

	private:

		// ASYNC - Prime context to write a message header
		void WriteHeader()
		{
			// If this function is called, we know the outgoing message queue must have 
			// at least one message to send. So allocate a transmission buffer to hold
			// the message, and issue the work - asio, send these bytes
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					// asio has now sent the bytes - if there was a problem
					// an error would be available...
					if (!ec)
					{
						// ... no error, so check if the message header just sent also
						// has a message body...
						if (m_qMessagesOut.front().body.size() > 0)
						{
							// ...it does, so issue the task to write the body bytes
							WriteBody();
						}
						else
						{
							// ...it didnt, so we are done with this message. Remove it from 
							// the outgoing message queue
							m_qMessagesOut.pop_front();

							// If the queue is not empty, there are more messages to send, so
							// make this happen by issuing the task to send the next header.
							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
					}
					else
					{
						// ...asio failed to write the message, we could analyse why but 
						// for now simply assume the connection has died by closing the
						// socket. When a future attempt to write to this client fails due
						// to the closed socket, it will be tidied up.
						std::cout << '[' << id << "] Write Header Fail.\n";
						m_socket.close();
					}
				});
		}

		// ASYNC - Prime context to write a message body
		void WriteBody()
		{
			// If this function is called, a header has just been sent, and that header
			// indicated a body existed for this message. Fill a transmission buffer
			// with the body data, and send it!
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// Sending was successful, so we are done with the message
						// and remove it from the queue
						m_qMessagesOut.pop_front();

						// If the queue still has messages in it, then issue the task to 
						// send the next messages' header.
						if (!m_qMessagesOut.empty())
						{
							WriteHeader();
						}
					}
					else
					{
						// Sending failed, see WriteHeader() equivalent for description :P
						std::cout << '[' << id << "] Write Body Fail.\n";
						m_socket.close();
					}
				});
		}

		// ASYNC - Prime context ready to read a message header
		void ReadHeader()
		{
			// If this function is called, we are expecting asio to wait until it receives
			// enough bytes to form a header of a message. We know the headers are a fixed
			// size, so allocate a transmission buffer large enough to store it. In fact, 
			// we will construct the message in a "temporary" message object as it's 
			// convenient to work with.
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// A complete message header has been read, check if this message
						// has a body to follow...
						if (m_msgTemporaryIn.header.size > 0)
						{
							// ...it does, so allocate enough space in the messages' body
							// vector, and issue asio with the task to read the body.
							m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
							ReadBody();
						}
						else
						{
							// it doesn't, so add this bodyless message to the connections
							// incoming message queue
							AddToIncomingMessageQueue();
						}
					}
					else
					{
						// Reading form the client went wrong, most likely a disconnect
						// has occurred. Close the socket and let the system tidy it up later.
						std::cout << '[' << id << "] Read Header Fail.\n";
						m_socket.close();
					}
				});
		}

		// ASYNC - Prime context ready to read a message body
		void ReadBody()
		{
			// If this function is called, a header has already been read, and that header
			// request we read a body, The space for that body has already been allocated
			// in the temporary message object, so just wait for the bytes to arrive...
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// ...and they have! The message is now complete, so add
						// the whole message to incoming queue
						AddToIncomingMessageQueue();
					}
					else
					{
						// As above!
						std::cout << '[' << id << "] Read Body Fail.\n";
						m_socket.close();
					}
				});
		}

		// Once a full message is received, add it to the incoming queue
		void AddToIncomingMessageQueue()
		{
			// Shove it in queue, converting it to an "owned message", by initialising
			// with the a shared pointer from this connection object
			if (m_nOwnerType == owner::server)
				m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
			else
				m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

			// We must now prime the asio context to receive the next message. It 
			// wil just sit and wait for bytes to arrive, and the message construction
			// process repeats itself. Clever huh?
			ReadHeader();
		}

		static uint64_t scramble(const uint64_t& nInput)
		{
			uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
			out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
			return out ^ 0xC0DEFACE12345678;
		}


		// ASYNC - Used by both client and server to write validation data
		void WriteValidation()
		{
			asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_nOwnerType == owner::client)
							ReadHeader();
					}
					else
					{
						m_socket.close();
					}
				}
			);
		}

		void ReadValidation(net::server_interface<T>* server = nullptr)
		{
			asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_nOwnerType == owner::server)
						{
							if (m_nHandshakeIn == m_nHandshakeCheck)
							{
								std::cout << "Client validated\n";
								server->OnClientValidated(this->shared_from_this());

								ReadHeader();
							}
							else
							{
								std::cout << "Client Disconnected (Fail Validation)\n";
								m_socket.close();
							}
						}
						else
						{
							m_nHandshakeOut = scramble(m_nHandshakeIn);

							WriteValidation();
						}
					}
					else
					{
						std::cout << "Client disconnected (Read Validation)\n";
						m_socket.close();
					}
				}
			);
		}

	protected:

		// The "owner" decides how some of the connection behaves
		owner m_nOwnerType = owner::server;

		// This context is shared with the whole asio instance
		asio::io_context& m_asioContext;

		// Each connection has a unique socket to a remote
		asio::ip::tcp::socket m_socket;

		// This queue holds all messages to be sent to the remote side
		// of this connection
		tsqueue<message<T>> m_qMessagesOut;

		// This references the incoming queue of the parent object
		tsqueue<owned_message<T>>& m_qMessagesIn;

		// Incoming messages are constructed asynchronously, so we will
// store the part assembled message here, until it is ready
		message<T> m_msgTemporaryIn;

		uint32_t id = 0;

		uint64_t m_nHandshakeOut	= 0;
		uint64_t m_nHandshakeIn		= 0;
		uint64_t m_nHandshakeCheck	= 0;
	};
}