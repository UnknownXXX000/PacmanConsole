// PacmanConsole.cpp: определяет точку входа для приложения.
//

#include "main.h"

using namespace std::chrono;


bool game_exit = false;

struct vector2di 
{
	int x = 0;
	int y = 0;
}; 

int main()
{
	asio::error_code ec;

	//Create a "context" - essentially the platform specific interface
	asio::io_context context;

	//Get the address of somewhere we wish to connect to
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34", ec), 80);

	asio::ip::tcp::socket socket(context);

	socket.connect(endpoint, ec);

	if (!ec)
		std::cout << "Connected\n";
	else
		std::cout << "Fail " << ec.message() << '\n';

	if (socket.is_open())
	{
		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

		const size_t bytes = socket.available();
		std::cout << "Bytes available: " << bytes << '\n';

		if (bytes > 0)
		{
			std::vector<char> vBuffer(bytes);
			socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

			for (const auto& c : vBuffer)
				std::cout << c;
		}
	}


	return 0;
}