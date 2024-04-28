#pragma once

#include "net_client.h"
#include "game_structs.h"

class GameClient : public net::client_interface<PTypes>
{
public:

	GameClient() = default;
	~GameClient() = default;

	void PingServer()
	{
		net::message<PTypes> msg;
		msg.header.id = PTypes::SERVER_PING;

		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	void MessageAll()
	{
		net::message<PTypes> msg;
		msg.header.id = PTypes::CLIENT_CONNECT;
		Send(msg);
	}
};