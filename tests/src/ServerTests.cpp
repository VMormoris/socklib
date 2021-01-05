#include <catch.hpp>

#include <cstring>
#define private public
#define protected public
#include <Server.h>


TEST_CASE("Server Constructor", "[Server]")
{
	{//Constructor with family argument
		TCP::Server server(AF_INET6);
		REQUIRE(server.m_Sock.GetNativeFD() != INVALID_SOCKET);
		REQUIRE(server.m_Sock.m_AF == AF_INET6);
	}

	{//Constructor with IP & port
		TCP::Server server(AF_INET, "127.0.0.1", 55555);
		REQUIRE(server.m_Sock.GetNativeFD() != INVALID_SOCKET);
		REQUIRE(server.m_Sock.m_AF == AF_INET);
	}

	{//Constructor with sockaddr_in
		sockaddr_in address{ 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
		TCP::Server server(address);
		REQUIRE(server.m_Sock.GetNativeFD() != INVALID_SOCKET);
		REQUIRE(server.m_Sock.m_AF == AF_INET);
	}

	{//Constructor with sockaddr_in6
		sockaddr_in6 address{ 0 };
		address.sin6_family = AF_INET6;
		address.sin6_port = htons(55555);
		address.sin6_addr = in6addr_any;
		TCP::Server server(address);
		REQUIRE(server.m_Sock.GetNativeFD() != INVALID_SOCKET);
		REQUIRE(server.m_Sock.m_AF == AF_INET6);
	}
}

TEST_CASE("Testing Server's Start()", "[Server]")
{
	const std::string expected = "Hello Server!";

	{//Using Client on function signature
		TCP::Server server(AF_INET);
		server.Bind("127.0.0.1", 55555);
		server.Listen();
		server.Start([&](TCP::Client client) {
			char buffer[1024];
			memset(buffer, 0, 1024);
			int bytes = client.Receive(buffer, 1024);
			REQUIRE(bytes == (expected.size() + 1));
			REQUIRE(expected.compare(buffer) == 0);
			REQUIRE(client.m_Sock.m_SockRef.use_count() == 2);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure server thread starts before procceding

		TCP::Client client(AF_INET);
		client.Connect("127.0.0.1", 55555);
		int bytes = client.Send(expected.c_str(), expected.size() + 1);

		client.Disconnect();
		
		REQUIRE(bytes == (expected.size() + 1));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		server.Stop();
		server.Join();
	}

	{//Using Socket on function signature
		TCP::Server server(AF_INET);
		server.Bind("127.0.0.1", 55555);
		server.Listen();
		server.Start([&](Socket sock) {
			char buffer[1024];
			memset(buffer, 0, 1024);
			int bytes = sock.Receive(buffer, 1024);
			REQUIRE(bytes == (expected.size() + 1));
			REQUIRE(expected.compare(buffer) == 0);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure server thread starts before procceding

		TCP::Client client(AF_INET);
		client.Connect("127.0.0.1", 55555);
		int bytes = client.Send(expected.c_str(), expected.size() + 1);

		client.Disconnect();

		REQUIRE(bytes == (expected.size() + 1));
		server.Stop();
		server.Join();
	}

	{//Using SOCKET on function signature
		TCP::Server server(AF_INET);
		server.Bind("127.0.0.1", 55555);
		server.Listen();
		server.Start([&](SOCKET sock) {
			char buffer[1024];
			memset(buffer, 0, 1024);
			int bytes = recv(sock, buffer, expected.size() + 1, 0);
			REQUIRE(bytes == (expected.size() + 1));
			REQUIRE(expected.compare(buffer) == 0);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure server thread starts before procceding

		TCP::Client client(AF_INET);
		client.Connect("127.0.0.1", 55555);
		int bytes = client.Send(expected.c_str(), expected.size() + 1);

		client.Disconnect();

		REQUIRE(bytes == (expected.size() + 1));
		server.Stop();
		server.Join();
	}

}

TEST_CASE("Testing Server's Start() with one spawn of Threads", "[Server]")
{
	const std::string expected = "Hello Server!";

	{//Using Client on function signature
		TCP::Server server(AF_INET);
		server.Bind("127.0.0.1", 55555);
		server.Listen();
		server.Start([&](TCP::Client client) {
			char buffer[1024];
			memset(buffer, 0, 1024);
			int bytes = client.Receive(buffer, 1024);
			REQUIRE(bytes == (expected.size() + 1));
			REQUIRE(expected.compare(buffer) == 0);
			REQUIRE(client.m_Sock.m_SockRef.use_count() == 2);
		}, 8);

		TCP::Client client(AF_INET);
		client.Connect("127.0.0.1", 55555);
		int bytes = client.Send(expected.c_str(), expected.size() + 1);

		client.Disconnect();

		REQUIRE(bytes == (expected.size() + 1));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		server.Stop();
		server.Join();
	}

	{//Using Socket on function signature
		TCP::Server server(AF_INET);
		server.Bind("127.0.0.1", 55555);
		server.Listen();
		server.Start([&](Socket sock) {
			char buffer[1024];
			memset(buffer, 0, 1024);
			int bytes = sock.Receive(buffer, 1024);
			REQUIRE(bytes == (expected.size() + 1));
			REQUIRE(expected.compare(buffer) == 0);
		}, 8);

		TCP::Client client(AF_INET);
		client.Connect("127.0.0.1", 55555);
		int bytes = client.Send(expected.c_str(), expected.size() + 1);

		client.Disconnect();

		REQUIRE(bytes == (expected.size() + 1));
		server.Stop();
		server.Join();
	}

	{//Using SOCKET on function signature
		TCP::Server server(AF_INET);
		server.Bind("127.0.0.1", 55555);
		server.Listen();
		server.Start([&](SOCKET sock) {
			char buffer[1024];
			memset(buffer, 0, 1024);
			int bytes = recv(sock, buffer, expected.size() + 1, 0);
			REQUIRE(bytes == (expected.size() + 1));
			REQUIRE(expected.compare(buffer) == 0);
		}, 8);

		TCP::Client client(AF_INET);
		client.Connect("127.0.0.1", 55555);
		int bytes = client.Send(expected.c_str(), expected.size() + 1);

		client.Disconnect();

		REQUIRE(bytes == (expected.size() + 1));
		server.Stop();
		server.Join();
	}

}