// PacmanConsole.cpp: определяет точку входа для приложения.
//

#include <includes.h>

//#include "game_client.h"
//#include "game_server.h"

using namespace std::chrono_literals;

class GameClient : public olc::PixelGameEngine, net::client_interface<PTypes>
{
public:
	GameClient()
	{
		sAppName = "PacmanConsole";
		myName = std::string("Default") + std::to_string(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
		serverIP = "127.0.0.1";
		serverPort = 60000;
		// myDirection = static_cast<PlayerMoves>(myName.size() % 4);
		// bRegisterMsgSent = false;

		if (myName.size() > 256u) myName.resize(256u);
	}
	GameClient(const std::string& name, const std::string& ip, const uint16_t& port) :
		myName(name), serverIP(ip), serverPort(port)
	{
		sAppName = "PacmanConsole";
		// bRegisterMsgSent = false;

		if (myName.size() > 256u) myName.resize(256u);
	}

private:

	inline void SendClientReady()
	{
		net::message<PTypes> clientReadyMsg;
		clientReadyMsg.header.id = PTypes::CLIENT_READY;
		// clientReadyMsg.SwapEndianness();
		
		clientReadyMsg.ReverseHeader();
		Send(clientReadyMsg);
	}

	inline void SendClientConnect()
	{
		// if (bRegisterMsgSent) return;

		net::message<PTypes> registerMsg;

		registerMsg.header.id = PTypes::CLIENT_CONNECT;
		char name[MAX_PLAYER_NAME_LEN];
		std::copy(myName.begin(), myName.end(), name);
		name[myName.size()] = '\0';
		registerMsg << name;

		registerMsg.ReverseHeader();
		// registerMsg.SwapEndianness();
		Send(registerMsg);
		std::cout << "Register message sent\n";

		// bRegisterMsgSent = true;
		// bRegisterMsgSent = true;
	}

	void SendKeyInput()
	{
		const auto endTime = std::chrono::system_clock::now();

		if (std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count() < 100ll) return;

		PlayerMoves dir = localPlayer->start_direction;

		int offset_x = 0, offset_y = 0;

		if (GetKey(olc::Key::W).bHeld) dir = PlayerMoves::UP;
		if (GetKey(olc::Key::S).bHeld) dir = PlayerMoves::DOWN;
		if (GetKey(olc::Key::A).bHeld) dir = PlayerMoves::LEFT;
		if (GetKey(olc::Key::D).bHeld) dir = PlayerMoves::RIGHT;

		switch (dir)
		{
		case PlayerMoves::UP:
			offset_y--;
			break;
		case PlayerMoves::DOWN:
			offset_y++;
			break;
		case PlayerMoves::LEFT:
			offset_x--;
			break;
		case PlayerMoves::RIGHT:
			offset_x++;
			break;
		}

		offset_x += static_cast<int>(localPlayer->start_x);
		offset_y += static_cast<int>(localPlayer->start_y);

		if (offset_x >= 0 && offset_x < GameMap.GetWidth() && offset_y >= 0 && offset_y < GameMap.GetHeight() &&
			(GameMap.at(offset_x, offset_y) == CellType::FOOD || GameMap.at(offset_x, offset_y) == CellType::EMPTY))
		{
			localPlayer->start_direction = dir;

			if (GameMap.at(offset_x, offset_y) == CellType::FOOD)
			{
				score++;
				foodLeft--;
			}

			GameMap(localPlayer->start_x, localPlayer->start_y) = CellType::EMPTY;

			localPlayer->start_x = static_cast<uint32_t>(offset_x);
			localPlayer->start_y = static_cast<uint32_t>(offset_y);

			GameMap(localPlayer->start_x, localPlayer->start_y) = CellType::PLAYER;


			net::message<PTypes> sendKeyMsg;
			sendKeyMsg.header.id = PTypes::CLIENT_SEND_KEY;

			sendKeyMsg << dir;

			sendKeyMsg.ReverseHeader();
			// sendKeyMsg.SwapEndianness();
			Send(sendKeyMsg);

			// std::this_thread::sleep_for(100ms);
			m_StartTime = std::chrono::system_clock::now();
		}
		
	}

public:
	bool OnUserCreate() override
	{
		tv = olc::TileTransformedView({ ScreenWidth(), ScreenHeight() }, { 8, 8 });
		if (Connect(serverIP, serverPort))
		{
			SendClientConnect();

			return true;
		}
		return false;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		if (!bGameOver && !IsConnected())
		{
			SendClientConnect();
			// std::this_thread::sleep_for(1s);
		}

		if (foodLeft == 0) bGameOver = true;
		if (bGameOver) 
		{
			std::cout << "Your score: " << score << std::endl;
			return false;
		}

		// std::cout << "On user update\n";
		if (IsConnected())
		{
			while (!Incoming().empty())
			{
				auto msg = Incoming().pop_front().msg;

				msg.ReverseHeader();

				// std::cout << std::hex << msg << '\n';

				if (msg.header.magic != MAGIC)
					continue;

				switch (msg.header.id)
				{
				case PTypes::SERVER_SEND_MAP:
				{
					bWaitingForRegistration = false;

					// if (!bRegisterMsgSent) bRegisterMsgSent = true;
					uint8_t arr[MAP_PART_SIZE];
					msg >> arr;
					FieldPart part;
					//part.ReadFromCArray(arr);
					part.ReadFromCArrayFixed(arr);

					std::cout << "Received map part:\n" << part << '\n';
					part.GenerateSymmetricMap(GameMap);

					SendClientReady();
					break;
				}
				case PTypes::SERVER_GAME_START:
				{
					bWaitingForRegistration = false;
					if (!bReady) 
					{
						m_StartTime = std::chrono::system_clock::now();
						bReady = true;
					}

					uint32_t frame_timeout = 0;
					size_t player_count = 4;

					msg >> frame_timeout;

					frame_timeout = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()) - frame_timeout;

					msg >> player_count;

					std::cout << "Frame timeout = " << frame_timeout << '\n';

					playerList.resize(player_count);

					for (size_t i = 0; i < player_count; i++)
					{
						player p;
						msg >> p;

						playerList[i].FromCPlayer(p);
						std::cout << playerList[i] << '\n';

						if (myName == playerList[i].name)
							localPlayer = &playerList[i];

						GameMap(playerList[i].start_x, playerList[i].start_y) = CellType::PLAYER;
					}

					foodLeft = GameMap.countFood();

					score = 0;

					break;
				}

				case PTypes::SERVER_SEND_KEY_TO_OTHERS:
				{
					PlayerMoves OtherDir;
					CppPlayer* playerSent = nullptr;
					char name[MAX_PLAYER_NAME_LEN];

					msg >> name;
					msg >> OtherDir;

					const std::string FromName(name);

					// std::cout << "Received direction " << static_cast<uint32_t>(OtherDir) << " from player " << FromName << '\n';

					for (auto& player : playerList)
					{
						if (player.name == FromName)
						{
							playerSent = &player;
							break;
						}
					}

					if (playerSent == nullptr) break;

					GameMap(playerSent->start_x, playerSent->start_y) = CellType::EMPTY;

					if		(OtherDir == PlayerMoves::UP)		playerSent->start_y--;
					else if (OtherDir == PlayerMoves::DOWN)		playerSent->start_y++;
					else if (OtherDir == PlayerMoves::LEFT)		playerSent->start_x--;
					else if (OtherDir == PlayerMoves::RIGHT)	playerSent->start_x++;

					if (GameMap.at(playerSent->start_x, playerSent->start_y) == CellType::FOOD)
						foodLeft--;

					GameMap(playerSent->start_x, playerSent->start_y) = CellType::PLAYER;

					break;
				}

				default:
					break;
				}
			}

			if (bWaitingForRegistration)
			{
				Clear(olc::DARK_BLUE);
				DrawString({ 10,10 }, "Waiting To Be Registered...", olc::WHITE);
				return true;
			}

			if (bReady) SendKeyInput();

			// Update players on map

			Clear(olc::BLACK);

			// Draw World
			olc::vi2d vTL = tv.GetTopLeftTile().max({ 0,0 });
			olc::vi2d vBR = tv.GetBottomRightTile().min(vWorldSize);
			olc::vi2d vTile;
			for (vTile.y = vTL.y; vTile.y < vBR.y; vTile.y++)
				for (vTile.x = vTL.x; vTile.x < vBR.x; vTile.x++)
				{
					const auto& currentTile = GameMap.at(vTile.x, vTile.y);
					switch (currentTile)
					{
					case CellType::WALL:
					{
						tv.FillRect(vTile, { 1.0f, 1.0f });
						// tv.DrawRect(olc::vf2d(vTile) + olc::vf2d(0.1f, 0.1f), { 0.8f, 0.8f });
						break;
					}
					case CellType::FOOD:
					{
						tv.FillCircle(olc::vf2d(vTile) + olc::vf2d(0.5f, 0.5f), 0.25f, olc::YELLOW);
						break;
					}
					case CellType::PLAYER:
					{
						if (vTile.x == localPlayer->start_x && vTile.y == localPlayer->start_y)
							tv.FillRect(olc::vf2d(vTile) + olc::vf2d(0.2f, 0.2f), {0.8f, 0.8f}, olc::GREEN);
						else
							tv.FillRect(olc::vf2d(vTile) + olc::vf2d(0.2f, 0.2f), { 0.8f, 0.8f }, olc::RED);
						break;
					}
					case CellType::EMPTY:
					{
						break;
					}

					}
				}

			// Draw other info

			DrawString({ 10, static_cast<int>(static_cast<float>(ScreenHeight()) * 0.8f) }, "Your score: " + std::to_string(score));
			DrawString({ 10, static_cast<int>(static_cast<float>(ScreenHeight()) * 0.7f) }, "Food left: " + std::to_string(foodLeft));

		}
		return true;
	}

private:
	olc::TileTransformedView tv;
	olc::vi2d vWorldSize = { 40, 30 };

	FieldMap GameMap;
	std::vector<CppPlayer> playerList;
	std::string myName;

	std::string serverIP;
	uint16_t serverPort;

	// PlayerMoves myDirection;
	bool bWaitingForRegistration = true;
	bool bReady = false;
	bool bGameOver = false;

	CppPlayer* localPlayer = nullptr;
	uint32_t score = 0;
	size_t foodLeft = UINT32_MAX;

	std::chrono::time_point<std::chrono::system_clock> m_StartTime;
	// std::chrono::time_point<std::chrono::system_clock> m_EndTime;
};


class GameServer : public net::server_interface<PTypes>
{
private:
	const uint16_t PlayerCount;
	std::unordered_map<uint32_t, CppPlayer> PlayerList;
	std::vector<uint32_t> BannedIDs;
	bool MapGenerated = false;
	FieldPart GenMap;
	uint32_t PlayerStartX;
	uint32_t PlayerStartY;
	std::unordered_set<uint32_t> ReadyClients;
	net::message<PTypes> SendMapMessage;
	// net::message<PTypes> SendPlayerListMessage;
public:

	GameServer(uint16_t port) : net::server_interface<PTypes>(port), PlayerCount(2), PlayerStartX(0), PlayerStartY(0)
	{
		PlayerList.reserve(PlayerCount);
		if (!MapGenerated)
		{
			GenMap.GenerateMap();
			FormSendMapMessage();
			GenMap.GetStartPosition(PlayerStartX, PlayerStartY);
			MapGenerated = true;
		}
	}

	GameServer(uint16_t port, uint32_t player_count) : net::server_interface<PTypes>(port), PlayerCount(player_count), PlayerStartX(0), PlayerStartY(0)
	{
		PlayerList.reserve(PlayerCount);
		if (!MapGenerated)
		{
			GenMap.GenerateMap();
			FormSendMapMessage();
			GenMap.GetStartPosition(PlayerStartX, PlayerStartY);
			MapGenerated = true;
		}
	}

private:
	inline void FormSendMapMessage()
	{
		// net::message<PTypes> mapMsg;
		SendMapMessage.header.id = PTypes::SERVER_SEND_MAP;

		uint8_t bytes[MAP_PART_SIZE];

		std::cout << "[GameServer] Generated map:\n" << GenMap << '\n';

		// GenMap.WriteToCArray(bytes);
		GenMap.WriteToCArrayFixed(bytes);

		SendMapMessage << bytes;

		SendMapMessage.ReverseHeader();

		// SendMapMessage.SwapEndianness();
		//const auto bytes = GenMap.ToBytes();

		//for (const auto& byte_ : bytes)
		//	SendMapMessage << byte_;

		// return mapMsg;
	}

	inline void FormSendPlayerListMessage()
	{
		// net::message<PTypes> playerListMsg;
		net::message<PTypes> SendPlayerListMessage;
		SendPlayerListMessage.header.id = PTypes::SERVER_GAME_START;

		for (const auto& [playerID, playerData] : PlayerList)
		{
			player p = playerData.ToCPlayer();
			SendPlayerListMessage << p;
		}

		SendPlayerListMessage << PlayerList.size();

		uint32_t frame_timeout = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());

		SendPlayerListMessage << frame_timeout;

		SendPlayerListMessage.ReverseHeader();
		// SendPlayerListMessage.SwapEndianness();

		MessageAllClients(SendPlayerListMessage);
		// return playerListMsg;
	}

	void OnGameReady()
	{
		RemoveDisconnectedClients();

		if (ReadyClients.size() == PlayerCount)
		{
			std::cout << "[GameServer] Game is ready. Sending player list\n";

			FormSendPlayerListMessage();

			// auto msg = FormSendPlayerListMessage();

			// MessageAllClients(msg);
		}
	}

protected:

	bool OnClientConnect(std::shared_ptr<net::connection<PTypes>> client) override
	{
		// RemoveDisconnectedClients();

		if ((PlayerList.size() + 1u > PlayerCount) || (std::find(BannedIDs.begin(), BannedIDs.end(), client->GetID()) != BannedIDs.end()))
		{
			DisconnectClient(client);
			return false;
		}

		std::cout << "[GameServer] client " << client->GetID() << " connected\n";

		//else
		//{
		//	std::cout << "[GameServer] client " << client->GetID() << " allowed\n";
		//	PlayerList[client->GetID()] = { 0, 0, PlayerMoves::UP, "Unnamed" };
		//}

		std::cout << "[GameServer] Sending map to player " << client->GetID() << '\n';
		MessageClient(client, SendMapMessage);


		//net::message<PTypes> msg;
		//msg.header.ptype = PTypes::SERVER_SEND_MAP;

		//uint8_t MapBytes[20 * 15];

		//GenMap.CopyToCArray(&MapBytes[0]);

		//msg << MapBytes;

		//client->Send(msg);
		return true;
	}

	void OnClientDisconnect(std::shared_ptr<net::connection<PTypes>> client) override
	{
		if (client)
		{
			std::cout << "[GameServer] client " << client->GetID() << " disconnected\n";

			const bool bBanned = std::find(BannedIDs.begin(), BannedIDs.end(), client->GetID()) != BannedIDs.end();

			if (bBanned || PlayerList.find(client->GetID()) == PlayerList.end())
			{
				// client never added to list, so just let it disappear
				if (bBanned)
					std::cout << "[GameServer] client " << client->GetID() << " is banned.\n";
			}

			else
			{
				auto& pd = PlayerList[client->GetID()];
				std::cout << "[GameServer] removed " << pd.name << '\n';
				PlayerList.erase(client->GetID());
				// PlayerCount--;
				// BannedIDs.push_back(client->GetID());
			}

			if (ReadyClients.find(client->GetID()) != ReadyClients.end())
			{
				std::cout << "[GameServer] client " << client->GetID() << " removed from ready clients\n";
				ReadyClients.erase(client->GetID());
			}
		}
	}

	// Called when a message arrives
	void OnMessage(std::shared_ptr<net::connection<PTypes>> client, net::message<PTypes>& msg) override
	{
		msg.ReverseHeader();
		// msg.SwapEndianness();
		// RemoveDisconnectedClients();

		// std::cout << "[GameServer] client: " << client->GetID() << ", message info: " << msg << '\n';

		//if (!BannedIDs.empty())
		//{
		//	for (const auto& pid : BannedIDs)
		//	{
		//		// olc::net::message<GameMsg> m;
		//		// m.header.id = GameMsg::Game_RemovePlayer;
		//		// m << pid;
		//		std::cout << "Removing " << pid << '\n';
		//		// MessageAllClients(m);
		//	}
		//	BannedIDs.clear();
		//}

		if (msg.header.magic != MAGIC)
		{
			DisconnectClient(client);
			return;
		}

		switch (msg.header.id)
		{
			//case PTypes::SERVER_PING:
			//{
			//	std::cout << '[' << client->GetID() << "]: Server Ping\n";

			//	// Simply bounce message back to client
			//	client->Send(msg);
			//}
			//break;

		case PTypes::CLIENT_CONNECT:
		{
			if (PlayerList.find(client->GetID()) != PlayerList.end() || std::find(BannedIDs.begin(), BannedIDs.end(), client->GetID()) != BannedIDs.end())
				break;

			std::cout << "[GameServer] [" << client->GetID() << "]: Client connected\n";

			auto& newPlayer = PlayerList[client->GetID()];
			// PlayerCount++;

			char tempName[MAX_PLAYER_NAME_LEN];

			msg >> tempName;

			newPlayer.name = std::string(tempName);

			if (newPlayer.name == "Server")
			{
				const auto ID = client->GetID();
				DisconnectClient(client);
				BannedIDs.push_back(ID);
				break;
			}

			bool bDone = false;

			for (const auto& [id, player] : PlayerList)
			{
				if (id != client->GetID() && newPlayer.name == player.name)
				{
					bDone = true;
					const auto ID = client->GetID();
					DisconnectClient(client);
					BannedIDs.push_back(ID);
					break;
				}
			}

			if (bDone) break;

			std::cout << "[GameServer] player id: " << client->GetID() << ", name: " << newPlayer.name << '\n';

			newPlayer.start_direction = static_cast<PlayerMoves>(newPlayer.name.size() % 4u);

			switch (PlayerList.size())
			{
			case 1u:
			{
				newPlayer.start_x = PlayerStartX;
				newPlayer.start_y = PlayerStartY;
				break;
			}
			case 2u:
			{
				newPlayer.start_x = 2u * GenMap.GetWidth()	- PlayerStartX - 1u;
				newPlayer.start_y = PlayerStartY;
				break;
			}
			case 3u:
			{
				newPlayer.start_x = 2u * GenMap.GetWidth()	- PlayerStartX - 1u;
				newPlayer.start_y = 2u * GenMap.GetHeight() - PlayerStartY - 1u;
				break;
			}
			case 4u:
			{
				newPlayer.start_x = PlayerStartX;
				newPlayer.start_y = 2u * GenMap.GetHeight() - PlayerStartY - 1u;
				break;
			}
			default:
				newPlayer.start_x = 0u;
				newPlayer.start_y = 0u;
				break;
			}

			// const auto mapMsg = FormSendMapMessage();


			//std::cout << "[GameServer] Sending map to player " << client->GetID() << '\n';
			//MessageClient(client, SendMapMessage);

			//if (PlayerList.size() == PlayerCount)
			//	OnGameReady();

			// SendMap(client);

			// Construct a new message and send it to all clients
			//net::message<PTypes> msg;
			//msg.header.ptype = PTypes::CLIENT_CONNECT;
			//msg << client->GetID();
			//MessageAllClients(msg, client);
			break;
		}

		case PTypes::CLIENT_READY:
		{
			std::cout << "[GameServer] player " << client->GetID() << ' ' << PlayerList.at(client->GetID()).name << " is ready!\n";
			// NumOfClientsReady++;
			ReadyClients.insert(client->GetID());
			if (ReadyClients.size() == PlayerCount)
				OnGameReady();
			break;
		}

		case PTypes::CLIENT_SEND_KEY:
		{
			PlayerMoves dir;
			msg >> dir;

			net::message<PTypes> SendKeyToOthersMsg;
			SendKeyToOthersMsg.header.id = PTypes::SERVER_SEND_KEY_TO_OTHERS;

			SendKeyToOthersMsg << dir;

			char name[MAX_PLAYER_NAME_LEN];

			std::copy(PlayerList.at(client->GetID()).name.begin(), PlayerList.at(client->GetID()).name.end(), name);
			name[PlayerList.at(client->GetID()).name.size()] = '\0';

			SendKeyToOthersMsg << name;

			SendKeyToOthersMsg.ReverseHeader();

			// SendKeyToOthersMsg.SwapEndianness();
			MessageAllClients(SendKeyToOthersMsg, client);

			break;
		}
		
		default:
		{
			std::cout << "[GameServer] [" << client->GetID() << "] wrong packet id. Removing client\n";
			DisconnectClient(client);
			break;
		}

		}
	}


};

//// Override base class with your custom functionality
//class Example : public olc::PixelGameEngine
//{
//public:
//	Example()
//	{
//		// Name your application
//		sAppName = "Example";
//	}
//
//public:
//	bool OnUserCreate() override
//	{
//		// Called once at the start, so create things here
//		return true;
//	}
//
//	bool OnUserUpdate(float fElapsedTime) override
//	{
//		// Called once per frame, draws random coloured pixels
//		for (int x = 0; x < ScreenWidth(); x++)
//			for (int y = 0; y < ScreenHeight(); y++)
//				Draw(x, y, olc::Pixel(rand() % 256, rand() % 256, rand() % 256));
//		return true;
//	}
//};


// using namespace std::chrono_literals;

//std::vector<char> vBuffer(10 * 1024);
//
//void GrabSomeData(asio::ip::tcp::socket& socket)
//{
//	// socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()));
//}



int main(int argc, char * argv[])
{

	argparse::ArgumentParser program("PacmanConsole");

	program.add_argument("-t", "--type").help("decide if a program is a client or a server").default_value<std::string>(std::string("client")).nargs(1);
	program.add_argument("-i", "--ip").help("if server, then decide the ip address the server will be running on. if client, then connect to a server with specific ip address").default_value<std::string>(std::string("127.0.0.1")).nargs(1);
	program.add_argument("-p", "--port").help("the server will be listening on specific port, or a client will connect to a server running on a specific port").default_value<uint16_t>(60000).scan <'d', uint16_t>().nargs(1);

	try
	{
		program.parse_args(argc, argv);
	}
	catch (const std::exception& err)
	{
		std::cerr << "Argparse failed. Error: " << err.what() << std::endl;
		std::cerr << program;
		return -1;
	}

	const auto program_type = program.get<std::string>("-t");
	const auto program_ip = program.get<std::string>("-i");
	const auto program_port = program.get<uint16_t>("-p");

	std::cout << "Program type is " << program_type << ", ip address is " << program_ip << " and port is " << program_port << std::endl;

	//net::message<net::PTypes> msg;

	//int a = 50, b = 40, c = 30;

	//std::cout << a << b << c << '\n';

	//msg << a << b << c;

	//a = b = c = 0;

	//std::cout << a << b << c << '\n';

	//msg >> c >> b >> a;

	//std::cout << a << b << c << '\n';

	//asio::error_code ec;

	////Create a "context" - essentially the platform specific interface
	//asio::io_context context;

	////Get the address of somewhere we wish to connect to
	//asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

	//asio::ip::tcp::socket socket(context);

	//socket.connect(endpoint, ec);

	//if (!ec)
	//	std::cout << "Connected\n";
	//else
	//	std::cout << "Fail " << ec.message() << '\n';

	//if (socket.is_open())
	//{
	//	std::string sRequest =
	//		"GET /index.html HTTP/1.1\r\n"
	//		"Host: example.com\r\n"
	//		"Connection: close\r\n\r\n";

	//	std::cout << "Number of bytes sent: " << socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec) << '\n';

	//	socket.wait(socket.wait_read);

	//	const size_t bytes = socket.available();
	//	std::cout << "Bytes available: " << bytes << '\n';

	//	if (bytes > 0)
	//	{
	//		std::vector<char> vBuffer(bytes);
	//		socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

	//		for (const auto& c : vBuffer)
	//			std::cout << c;
	//	}
	//}


	return 0;
}