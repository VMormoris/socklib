#include <catch.hpp>

#define private public
#define protected public
#include <Client.h>

TEST_CASE("Testing BasicClient's Default Construction", "[BasicClient]")
{
	BasicClient client;
	REQUIRE(client.m_Sock.GetNativeFD() == INVALID_SOCKET);
	REQUIRE(client.m_State == BasicClient::ThreadState::None);
}

TEST_CASE("Testing Start()", "[BasicClient]")
{
	{//Detached Thread & Socket
		BasicClient client(AF_INET, SOCK_STREAM);
		SOCKET fd = client.GetSocket().GetNativeFD();
		client.Start([fd](Socket sock) {
			REQUIRE(fd == sock.GetNativeFD());
		}, true);
	}

	{//Attached Thread & Socket
		BasicClient client(AF_INET, SOCK_STREAM);
		client.Start([&](Socket sock) {
			REQUIRE(client.GetSocket().GetNativeFD() == sock.GetNativeFD());
		});
		client.Join();
	}

	{//Detached Thread & SOCKET
		BasicClient client(AF_INET, SOCK_STREAM);
		SOCKET fd = client.GetSocket().GetNativeFD();
		client.Start([fd](SOCKET sock) {
			REQUIRE(fd == sock);
			closesocket(sock);
		}, true);
	}

	{//Attached Thread & SOCKET
		BasicClient client(AF_INET, SOCK_STREAM);
		client.Start([&](SOCKET sock) {
			REQUIRE(client.GetSocket().GetNativeFD() == sock);
		});
		client.Join();
	}
	
}
