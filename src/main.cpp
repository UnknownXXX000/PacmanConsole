// PacmanConsole.cpp: определяет точку входа для приложения.
//

#include "main.h"


using namespace std::chrono;
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