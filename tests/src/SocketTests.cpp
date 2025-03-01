#include <catch.hpp>

#include <future>

#include <socklib/Socket.h>

#include <thread>
using namespace socklib;

TEST_CASE("Testing Default Constructor", "[Socket]")
{
	const Socket sock;
	REQUIRE(sock.FileNo() == INVALID_SOCKET);
	REQUIRE(sock.IsBlocking() == true);//By default sockets are in blocking mode
}

TEST_CASE("Testing Copy Constructor", "[Socket]")
{
	const Socket sock;
	const Socket copy(sock);
	REQUIRE(sock.FileNo() == copy.FileNo());
	REQUIRE(sock.IsBlocking() == copy.IsBlocking());
}

TEST_CASE("Testing Move Constructor", "[Socket]")
{
	{
		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		Socket moved(std::move(sock));

		REQUIRE(sock.FileNo() == INVALID_SOCKET);
		REQUIRE(sock.FileNo() != moved.FileNo());
		REQUIRE(sock.IsBlocking() == moved.IsBlocking());
	}

	{
		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		Socket copy(sock);
		Socket moved(std::move(sock));

		REQUIRE(sock.FileNo() == INVALID_SOCKET);
		REQUIRE(sock.FileNo() != moved.FileNo());
		REQUIRE(copy.FileNo() == moved.FileNo());
		REQUIRE(sock.IsBlocking() == sock.IsBlocking());
	}
}

TEST_CASE("Testing Open()", "[Socket]")
{
	{//IPv4 Stream Socket
		Socket sock;
		sock.Open(AddressFamily::IPv4, SocketType::STREAM);
		REQUIRE(sock.FileNo() != INVALID_SOCKET);
	}

	{//IPv4 Datagram Socket
		Socket sock;
		sock.Open(AddressFamily::IPv4, SocketType::DGRAM);
		REQUIRE(sock.FileNo() != INVALID_SOCKET);
	}

	{//IPv6 Stream Socket
		Socket sock;
		sock.Open(AddressFamily::IPv6, SocketType::STREAM);
		REQUIRE(sock.FileNo() != INVALID_SOCKET);
	}

	{//IPv6 Datagram Socket
		Socket sock;
		sock.Open(AddressFamily::IPv6, SocketType::DGRAM);
		REQUIRE(sock.FileNo() != INVALID_SOCKET);
	}
}

TEST_CASE("Testing Close()", "[Socket]")
{
	Socket sock;
	sock.Open(AddressFamily::IPv4, SocketType::STREAM);
	sock.Close();
	REQUIRE(sock.FileNo() == INVALID_SOCKET);
}

TEST_CASE("Testing Bind()", "Socket")
{
	{//Binding using sockaddr_in
		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		sock.Bind(reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in));

		sockaddr_in o_address = { 0 };
		socklen_t length = sizeof(sockaddr_in);


		getsockname(sock.FileNo(), reinterpret_cast<sockaddr*>(&o_address), &length);

		REQUIRE(address.sin_port == o_address.sin_port);
		REQUIRE(address.sin_addr.s_addr == o_address.sin_addr.s_addr);
	}

	{//Binding using sockaddr_in6
		sockaddr_in6 address = { 0 };
		address.sin6_family = AF_INET6;
		address.sin6_port = htons(55555);
		inet_pton(AF_INET6, "::1", &address.sin6_addr);

		Socket sock(AddressFamily::IPv6, SocketType::DGRAM);
		sock.Bind(reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in6));

		sockaddr_in6 o_address = { 0 };
		socklen_t length = sizeof(sockaddr_in6);

		getsockname(sock.FileNo(), reinterpret_cast<sockaddr*>(&o_address), &length);
		REQUIRE(address.sin6_port == o_address.sin6_port);
		for (size_t i = 0; i < 16; i++)
		{
			REQUIRE(address.sin6_addr.s6_addr[i] == o_address.sin6_addr.s6_addr[i]);
		}
	}

	{//Binding using human-readable pair of IP and port
		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		sock.Bind("127.0.0.1", 55555);

		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

		sockaddr_in o_address = { 0 };
		socklen_t length = sizeof(sockaddr_in);

		getsockname(sock.FileNo(), reinterpret_cast<sockaddr*>(&o_address), &length);

		REQUIRE(address.sin_port == o_address.sin_port);
		REQUIRE(address.sin_addr.s_addr == o_address.sin_addr.s_addr);
	}
}

TEST_CASE("Testing Listen()", "[Socket]")
{
	const Socket sock(AddressFamily::IPv4, SocketType::STREAM);
	sock.Bind();
	sock.Listen();
}

#include <iostream>

TEST_CASE("Testing Accept()", "Socket")
{
	{//Accept without address argument
		auto task = std::async(std::launch::async, []()
		{
			const Socket sock(AddressFamily::IPv4, SocketType::STREAM);
			sock.Bind("127.0.0.1", 55555);
			sock.Listen();
			auto [client, address] = sock.Accept();
			const auto& [ip, port] = address;
			REQUIRE(client.FileNo() != INVALID_SOCKET);
			REQUIRE(ip == "127.0.0.1");
			REQUIRE(port == 55556);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		sock.Bind("127.0.0.1", 55556);
		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

		int result = connect(sock.FileNo(), reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in));
		REQUIRE(result != SOCKET_ERROR);

		task.wait();
	}

	{//Accept IPv4
		sockaddr_in clientAddress = { 0 };
		clientAddress.sin_family = AF_INET;
		clientAddress.sin_port = htons(55556);
		inet_pton(AF_INET, "127.0.0.1", &clientAddress.sin_addr);

		auto task = std::async(std::launch::async, [clientAddress]()
		{
			const Socket sock(AddressFamily::IPv4, SocketType::STREAM);
			sock.Bind("127.0.0.1", 55565);
			sock.Listen();
			sockaddr_in address = { 0 };
			socklen_t addressSize = sizeof(sockaddr_in);
			const Socket client = sock.Accept(reinterpret_cast<sockaddr*>(&address), &addressSize);

			REQUIRE(client.FileNo() != INVALID_SOCKET);

			REQUIRE(clientAddress.sin_port == address.sin_port);
			REQUIRE(clientAddress.sin_addr.s_addr == address.sin_addr.s_addr);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding


		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		sock.Bind(reinterpret_cast<sockaddr*>(&clientAddress), sizeof(sockaddr_in));

		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55565);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

		int result = connect(sock.FileNo(), reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in));
		REQUIRE(result != SOCKET_ERROR);

		task.wait();
	}

	{//Accept IPv6
		sockaddr_in6 clientAddress = { 0 };
		clientAddress.sin6_family = AF_INET6;
		clientAddress.sin6_port = htons(55556);
		inet_pton(AF_INET6, "::1", &clientAddress.sin6_addr);
		auto task = std::async(std::launch::async, [clientAddress]()
		{
			const Socket sock(AddressFamily::IPv6, SocketType::STREAM);
			sock.Bind("::1", 55555);
			sock.Listen();
			sockaddr_in6 address = { 0 };
			socklen_t addressSize = sizeof(sockaddr_in6);
			const Socket client = sock.Accept(reinterpret_cast<sockaddr*>(&address), &addressSize);

			REQUIRE(client.FileNo() != INVALID_SOCKET);

			REQUIRE(clientAddress.sin6_port == address.sin6_port);
			for (size_t i = 0; i < 16; i++)
			{
				REQUIRE(clientAddress.sin6_addr.s6_addr[i] == address.sin6_addr.s6_addr[i]);
			}
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		Socket sock(AddressFamily::IPv6, SocketType::STREAM);
		sock.Bind(reinterpret_cast<sockaddr*>(&clientAddress), sizeof(sockaddr_in6));

		sockaddr_in6 address = { 0 };
		address.sin6_family = AF_INET6;
		address.sin6_port = htons(55555);
		inet_pton(AF_INET6, "::1", &address.sin6_addr);

		int result = connect(sock.FileNo(), reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in6));
		REQUIRE(result != SOCKET_ERROR);

		task.wait();
	}
}


TEST_CASE("Testing Connect()", "[Socket]")
{
	{//Connect using sockaddr_in
		const auto task = std::async(std::launch::async, []()
		{
			const Socket sock(AddressFamily::IPv4, SocketType::STREAM);
			sock.Bind("127.0.0.1", 55575);
			sock.Listen();
			auto [client, address] = sock.Accept();
			const auto& [ip, port] = address;
			REQUIRE(client.FileNo() != INVALID_SOCKET);
			REQUIRE(ip == "127.0.0.1");
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		const Socket client(AddressFamily::IPv4, SocketType::STREAM);

		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55575);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
		client.Connect(reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in));

		task.wait();
	}

	{//Connect using sockaddr_in6

		const auto task = std::async(std::launch::async, []()
		{
			const Socket sock(AddressFamily::IPv6, SocketType::STREAM);
			sock.Bind("::1", 55565);
			sock.Listen();
			auto [client, address] = sock.Accept();
			const auto& [ip, port] = address;
			REQUIRE(client.FileNo() != INVALID_SOCKET);
			REQUIRE(ip == "::1");
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		const Socket client(AddressFamily::IPv6, SocketType::STREAM);

		sockaddr_in6 address = { 0 };
		address.sin6_family = AF_INET6;
		address.sin6_port = htons(55565);
		inet_pton(AF_INET6, "::1", &address.sin6_addr);

		client.Connect(reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in6));

		task.wait();
	}

	{//Connect using the python way (IPv4)
		const auto task = std::async(std::launch::async, []()
		{
			const Socket sock(AddressFamily::IPv4, SocketType::STREAM);
			sock.Bind("127.0.0.1", 55585);
			sock.Listen();
			auto [client, address] = sock.Accept();
			const auto& [ip, port] = address;
			REQUIRE(client.FileNo() != INVALID_SOCKET);
			REQUIRE(ip == "127.0.0.1");
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		const Socket client(AddressFamily::IPv4, SocketType::STREAM);
		client.Connect("127.0.0.1", 55585);

		task.wait();
	}

	{//Connect using the python way (IPv6)
		const auto task = std::async(std::launch::async, []()
		{
			const Socket sock(AddressFamily::IPv6, SocketType::STREAM);
			sock.Bind("::1", 55575);
			sock.Listen();
			auto [client, address] = sock.Accept();
			const auto& [ip, port] = address;
			REQUIRE(client.FileNo() != INVALID_SOCKET);
			REQUIRE(ip == "::1");
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		const Socket client(AddressFamily::IPv6, SocketType::STREAM);
		client.Connect("::1", 55575);

		task.wait();
	}
}


TEST_CASE("Testing Datagram Transmission", "[Socket]")
{

	{//IPv4
		sockaddr_in senderAddress = { 0 };
		senderAddress.sin_family = AF_INET;
		senderAddress.sin_port = htons(55556);
		inet_pton(AF_INET, "127.0.0.1", &senderAddress.sin_addr);

		auto task = std::async(std::launch::async, [=]()
		{
			Socket sock(AddressFamily::IPv4, SocketType::DGRAM);
			sock.Bind("127.0.0.1", 55555);

			char buffer[12];

			sockaddr_in address = { 0 };
			socklen_t addressSize = sizeof(sockaddr_in);
			IOSize bytes = sock.ReceiveFrom(buffer, reinterpret_cast<sockaddr*>(&address), &addressSize, 6);
			std::string expected = "Hello";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected == buffer);
			REQUIRE(address.sin_port == senderAddress.sin_port);
			REQUIRE(address.sin_addr.s_addr == senderAddress.sin_addr.s_addr);

			address = { 0 };
			bytes = sock.ReceiveFrom(buffer, reinterpret_cast<sockaddr*>(&address), &addressSize, 6, 6);
			expected = "World";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected == &buffer[6]);
			REQUIRE(address.sin_port == senderAddress.sin_port);
			REQUIRE(address.sin_addr.s_addr == senderAddress.sin_addr.s_addr);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		constexpr char str[12] = "Hello\0World";

		Socket sock(AddressFamily::IPv4, SocketType::DGRAM);
		sock.Bind(reinterpret_cast<sockaddr *>(&senderAddress), sizeof(sockaddr_in6));

		sockaddr_in recverAddress = { 0 };
		recverAddress.sin_family = AF_INET;
		recverAddress.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &recverAddress.sin_addr);

		IOSize bytes = sock.SendTo(str, reinterpret_cast<sockaddr *>(&recverAddress), sizeof(sockaddr_in), 6);
		REQUIRE(bytes == 6);
		bytes = sock.SendTo(str, reinterpret_cast<sockaddr*>(&recverAddress), sizeof(sockaddr_in), 6, 6);
		REQUIRE(bytes == 6);

		task.wait();
	}

	{//IPv6
		sockaddr_in6 senderAddress = { 0 };
		senderAddress.sin6_family = AF_INET6;
		senderAddress.sin6_port = htons(55556);
		inet_pton(AF_INET6, "::1", &senderAddress.sin6_addr);

		auto task = std::async(std::launch::async, [=]()
		{
			Socket sock(AddressFamily::IPv6, SocketType::DGRAM);
			sock.Bind("::1", 55555);

			char buffer[12];

			sockaddr_in6 address = { 0 };
			socklen_t addressSize = sizeof(sockaddr_in6);
			IOSize bytes = sock.ReceiveFrom(buffer, reinterpret_cast<sockaddr*>(&address), &addressSize, 6);
			std::string expected = "Hello";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected == buffer);
			REQUIRE(address.sin6_port == senderAddress.sin6_port);
			for (size_t i = 0; i < 16; i++)
			{
				REQUIRE(address.sin6_addr.s6_addr[i] == senderAddress.sin6_addr.s6_addr[i]);
			}

			address = { 0 };
			bytes = sock.ReceiveFrom(buffer, reinterpret_cast<sockaddr*>(&address), &addressSize, 6, 6);
			expected = "World";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected == &buffer[6]);
			REQUIRE(address.sin6_port == senderAddress.sin6_port);
			for (size_t i = 0; i < 16; i++)
			{
				REQUIRE(address.sin6_addr.s6_addr[i] == senderAddress.sin6_addr.s6_addr[i]);
			}
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		constexpr char str[12] = "Hello\0World";

		Socket sock(AddressFamily::IPv6, SocketType::DGRAM);
		sock.Bind(reinterpret_cast<sockaddr*>(&senderAddress), sizeof(sockaddr_in6));

		sockaddr_in6 recverAddress = { 0 };
		recverAddress.sin6_family = AF_INET6;
		recverAddress.sin6_port = htons(55555);
		inet_pton(AF_INET6, "::1", &recverAddress.sin6_addr);

		IOSize bytes = sock.SendTo(str, reinterpret_cast<sockaddr*>(&recverAddress), sizeof(sockaddr_in6), 6);
		REQUIRE(bytes == 6);
		bytes = sock.SendTo(str, reinterpret_cast<sockaddr*>(&recverAddress), sizeof(sockaddr_in6), 6, 6);
		REQUIRE(bytes == 6);

		task.wait();
	}

	{//High level IO with datagram
		auto task = std::async(std::launch::async, []() {
			Socket server(AddressFamily::IPv4, SocketType::DGRAM, 0);
			server.Bind("127.0.0.1", 55555);

			char buffer[12];
			{
				auto [bytes, client] = server.ReceiveFrom(buffer, 6);
				const auto& [ip, port] = client;
				std::string expected = "Hello";
				REQUIRE(bytes == expected.size() + 1);
				REQUIRE(expected == buffer);
				REQUIRE(ip == "127.0.0.1");
				REQUIRE(port == 55556);
			}

			{
				auto [bytes, client] = server.ReceiveFrom(buffer, 6, 6);
				const auto& [ip, port] = client;
				std::string expected = "World";
				REQUIRE(bytes == expected.size() + 1);
				REQUIRE(expected == &buffer[6]);
				REQUIRE(ip == "127.0.0.1");
				REQUIRE(port == 55556);
			}

		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

		constexpr char str[12] = "Hello\0World";

		Socket sock(AddressFamily::IPv4, SocketType::DGRAM);
		sock.Bind("127.0.0.1", 55556);
		IOSize bytes = sock.SendTo(str, {"127.0.0.1", 55555}, 6);
		REQUIRE(bytes == 6);
		bytes = sock.SendTo(str, {"127.0.0.1", 55555}, 6, 6);
		REQUIRE(bytes == 6);

		task.wait();

	}
}

TEST_CASE("Testing static functions", "[Socket]")
{
	const auto task = std::async(std::launch::async, [](){
		const Socket server = Socket::CreateServer(AddressFamily::IPv4, { "127.0.0.1", 55605 });
		auto [client, endpoint] = server.Accept();
		const auto& [host, port] = endpoint;

		REQUIRE(client.FileNo() != INVALID_SOCKET);
		REQUIRE(host == "127.0.0.1");
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

	Socket client = Socket::CreateConnection(AddressFamily::IPv4, {"127.0.0.1", 55605});

	task.wait();
}


TEST_CASE("Testing Copy assignment operator", "[Socket]")
{
	const Socket sock;
	Socket copy; copy = sock;
	REQUIRE(sock.FileNo() == copy.FileNo());
	REQUIRE(sock.IsBlocking() == copy.IsBlocking());
}

TEST_CASE("Testing Move assignment operator", "[Socket]")
{
	{
		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		Socket moved; moved=std::move(sock);

		REQUIRE(sock.FileNo() == INVALID_SOCKET);
		REQUIRE(sock.FileNo() != moved.FileNo());
		REQUIRE(sock.IsBlocking() == moved.IsBlocking());
	}

	{
		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		Socket copy(sock);
		Socket moved; moved = std::move(sock);

		REQUIRE(sock.FileNo() == INVALID_SOCKET);
		REQUIRE(sock.FileNo() != moved.FileNo());
		REQUIRE(copy.FileNo() == moved.FileNo());
		REQUIRE(sock.IsBlocking() == moved.IsBlocking());
	}
}

TEST_CASE("Accept timeout", "[Socket]")
{
	const Socket sock(AddressFamily::IPv4, SocketType::STREAM);
	sock.SetTimeout(10);
	sock.Listen();
	auto [clientSock, endpoint] = sock.Accept();
	const auto& [host, port] = endpoint;
	REQUIRE(clientSock.FileNo() == INVALID_SOCKET);
}

TEST_CASE("Receive timeout", "[Socket]")
{
	const Endpoint endpoint("127.0.0.1", 55555);
	const auto task = std::async(std::launch::async, [endpoint](){
		const Socket server = Socket::CreateServer(AddressFamily::IPv4, endpoint);
		auto [client, endpoint] = server.Accept();
		const auto& [host, port] = endpoint;

		REQUIRE(client.FileNo() != INVALID_SOCKET);
		REQUIRE(host == "127.0.0.1");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));//Make sure the task starts before proceeding
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

	const Socket sock(AddressFamily::IPv4, SocketType::STREAM);
	sock.SetTimeout(10);
	sock.Connect(endpoint);

	char buffer[1024] = { 0 };
	const IOSize bytes = sock.Receive(buffer, 1024);

	REQUIRE(bytes == -1);
	task.wait();
}

TEST_CASE("Send timeout", "[Socket]")
{
	const Endpoint endpoint("127.0.0.1", 55555);
	const auto task = std::async(std::launch::async, [endpoint](){
		const Socket server = Socket::CreateServer(AddressFamily::IPv4, endpoint);
		auto [client, endpoint] = server.Accept();
		const auto& [host, port] = endpoint;

		REQUIRE(client.FileNo() != INVALID_SOCKET);
		REQUIRE(host == "127.0.0.1");

		char buffer[MiB] = { 0 };
		client.Receive(buffer, 1);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));//Make sure the task starts before proceeding
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

	const Socket sock(AddressFamily::IPv4, SocketType::STREAM);
	sock.SetTimeout(10);
	sock.Connect(endpoint);

	constexpr char buffer[32 * KiB] = { 0 };
	const IOSize bytes = sock.Send(buffer, 32 * KiB);

	REQUIRE(bytes == -1);
	task.wait();
}

TEST_CASE("SendTo timeout", "[Socket]")
{
	const Endpoint endpoint("127.0.0.1", 55555);
	const auto task = std::async(std::launch::async, [endpoint](){
		const Socket server { AddressFamily::IPv4, SocketType::DGRAM };
		server.Bind(endpoint);
		char buffer[MiB] = { 0 };

		std::this_thread::sleep_for(std::chrono::milliseconds(100));//Make sure the task starts before proceeding
		auto [bytes, sender] = server.ReceiveFrom(buffer, 1024);
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

	const Socket sock(AddressFamily::IPv4, SocketType::DGRAM);
	sock.SetTimeout(100);

	constexpr char buffer[1024] = { 0 };
	const IOSize bytes = sock.SendTo(buffer, endpoint,1024);

	REQUIRE(bytes == 1024);
	task.wait();
}

TEST_CASE("Readme.md checks", "[Socket]")
{
	const auto task = std::async(std::launch::async, []{// Server
		Socket server(AddressFamily::IPv4, SocketType::STREAM);
		server.Bind("127.0.0.1", 55555);
		server.Listen();
		char buffer[KiB];

		auto [sock, client] = server.Accept();
		const auto& [address, port] = client;

		bool running = true;
		while (running)
		{
			IOSize bytes = sock.Receive(buffer, KiB);
			bytes = sock.Send(buffer, bytes);
			if (std::string("/bye") == buffer)
				running = false;
		}
		sock.Close();
		server.Close();
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before proceeding

	{
		constexpr char msg[] = "Hello from socklib";
		char buffer[KiB];
		Socket sock(AddressFamily::IPv4, SocketType::STREAM);
		sock.Connect("127.0.0.1", 55555);
		IOSize bytes = sock.Send(msg, 19);
		bytes = sock.Receive(buffer, KiB);
		bytes = sock.Send("/bye", 5);
		bytes = sock.Receive(buffer, KiB);
		sock.Shutdown(SHUT_RDWR);
		sock.Close();
	}

	task.wait();
}