#pragma once

#include "net_connection.h"
#include "net_server.h"
#include "game_structs.h"

class GameServer : public net::server_interface<PTypes>
{
public:

	GameServer(uint16_t port) : net::server_interface<PTypes>(port)
	{
	}
protected:

	bool OnClientConnect(std::shared_ptr<net::connection<PTypes>> client) override
	{
		net::message<PTypes> msg;
		msg.header.id = PTypes::SERVER_ACCEPT;

		client->Send(msg);
		return true;
	}

	void OnClientDisconnect(std::shared_ptr<net::connection<T>> client) override
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	void OnMessage(std::shared_ptr<net::connection<PTypes>> client, net::message<PTypes>& msg) override
	{
		switch (msg.header.id)
		{
		case PTypes::SERVER_PING:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			// Simply bounce message back to client
			client->Send(msg);
		}
		break;

		case PTypes::CLIENT_CONNECT:
		{
			std::cout << "[" << client->GetID() << "]: Client connected\n";

			// Construct a new message and send it to all clients
			net::message<PTypes> msg;
			msg.header.id = PTypes::CLIENT_CONNECT;
			msg << client->GetID();
			MessageAllClients(msg, client);

		}
		break;
		}
	}


};