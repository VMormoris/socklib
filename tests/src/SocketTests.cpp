#include <catch.hpp>

#include <future>

#define private public
#define protected public
#include <socklib/socket.h>

TEST_CASE("Testing Default Contructor", "[Socket]")
{
	Socket sock;
	REQUIRE(sock.GetNativeFD() == INVALID_SOCKET);
	REQUIRE(sock.m_AF == AF_UNSPEC);
	REQUIRE(sock.m_BlockMode == true);//By default sockets are in blocking mode
	REQUIRE(sock.m_SockRef.use_count() == 1);
}

TEST_CASE("Testing Copy Constructor", "[Socket]")
{
	Socket sock;
	Socket copy(sock);
	REQUIRE(sock.GetNativeFD() == copy.GetNativeFD());
	REQUIRE(sock.m_AF == copy.m_AF);
	REQUIRE(sock.m_BlockMode == copy.m_BlockMode);
	REQUIRE(sock.m_SockRef.use_count() == 2);
}

TEST_CASE("Testing Move Constructor", "[Socket]")
{
	{
		Socket sock(AF_INET, SOCK_STREAM);
		Socket moved(std::move(sock));

		REQUIRE(sock.GetNativeFD() == INVALID_SOCKET);
		REQUIRE(sock.GetNativeFD() != moved.GetNativeFD());
		REQUIRE(sock.m_AF != moved.m_AF);
		REQUIRE(sock.m_AF == AF_UNSPEC);
		REQUIRE(sock.m_BlockMode == moved.m_BlockMode);
		REQUIRE(sock.m_SockRef.use_count() == 1);
		REQUIRE(moved.m_SockRef.use_count() == 1);
	}

	{
		Socket sock(AF_INET, SOCK_STREAM);
		Socket copy(sock);
		Socket moved(std::move(sock));

		REQUIRE(sock.GetNativeFD() == INVALID_SOCKET);
		REQUIRE(sock.GetNativeFD() != moved.GetNativeFD());
		REQUIRE(copy.GetNativeFD() == moved.GetNativeFD());
		REQUIRE(sock.m_AF != moved.m_AF);
		REQUIRE(moved.m_AF == copy.m_AF);
		REQUIRE(sock.m_AF == AF_UNSPEC);
		REQUIRE(sock.m_BlockMode == moved.m_BlockMode);
		REQUIRE(sock.m_SockRef.use_count() == 1);
		REQUIRE(moved.m_SockRef.use_count() == 2);
	}
}

TEST_CASE("Testing Open()", "[Socket]")
{
	{//IPv4 Stream Socket
		Socket sock;
		sock.Open(AF_INET, SOCK_STREAM);
		REQUIRE(sock.GetNativeFD() != INVALID_SOCKET);
		REQUIRE(sock.m_AF == AF_INET);
	}

	{//IPv4 Datagram Socket
		Socket sock;
		sock.Open(AF_INET, SOCK_DGRAM);
		REQUIRE(sock.GetNativeFD() != INVALID_SOCKET);
		REQUIRE(sock.m_AF == AF_INET);
	}

	{//IPv6 Stream Socket
		Socket sock;
		sock.Open(AF_INET6, SOCK_STREAM);
		REQUIRE(sock.GetNativeFD() != INVALID_SOCKET);
		REQUIRE(sock.m_AF == AF_INET6);
	}

	{//IPv6 Datagram Socket
		Socket sock;
		sock.Open(AF_INET6, SOCK_DGRAM);
		REQUIRE(sock.GetNativeFD() != INVALID_SOCKET);
		REQUIRE(sock.m_AF == AF_INET6);
	}
}

TEST_CASE("Testing Close()", "[Socket]")
{
	Socket sock;
	sock.Open(AF_INET, SOCK_STREAM);
	sock.Close();
	REQUIRE(sock.GetNativeFD() == INVALID_SOCKET);
	REQUIRE(sock.m_AF == AF_UNSPEC);
}

TEST_CASE("Testing Bind()", "Socket")
{
	{//Binding using sockaddr_in
		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
		
		Socket sock(AF_INET, SOCK_STREAM);
		sock.Bind(address);

		sockaddr_in o_address = { 0 };

	#ifdef PLATFORM_WINDOWS
		int length = sizeof(sockaddr_in);
	#else//Unix like platform
		socklen_t length = sizeof(sockaddr_in);
	#endif

		getsockname(sock.GetNativeFD(), (sockaddr*)&o_address, &length);
		
		REQUIRE(address.sin_port == o_address.sin_port);
	#ifdef PLATFORM_WINDOWS
		REQUIRE(address.sin_addr.S_un.S_addr == o_address.sin_addr.S_un.S_addr);
	#else//Unix like platform
		REQUIRE(address.sin_addr.s_addr == o_address.sin_addr.s_addr);
	#endif
	}

	{//Binding using sockaddr_in6
		sockaddr_in6 address = { 0 };
		address.sin6_family = AF_INET6;
		address.sin6_port = htons(55555);
		inet_pton(AF_INET, "::1", &address.sin6_addr);

		Socket sock(AF_INET6, SOCK_DGRAM);
		sock.Bind(address);

		sockaddr_in6 o_address = { 0 };
		
	#ifdef PLATFORM_WINDOWS
		int length = sizeof(sockaddr_in6);
	#else//Unix like platform
		socklen_t length = sizeof(sockaddr_in6);
	#endif

		getsockname(sock.GetNativeFD(), (sockaddr*)&o_address, &length);
		REQUIRE(address.sin6_port == o_address.sin6_port);
		for (size_t i = 0; i < 16; i++)
		{
		#ifdef PLATFORM_WINDOWS
			REQUIRE(address.sin6_addr.u.Byte[i] == o_address.sin6_addr.u.Byte[i]);
		#else//Unix like platforms
			REQUIRE(address.sin6_addr.s6_addr[i] == o_address.sin6_addr.s6_addr[i]);
		#endif
		}
	}

	{//Binding using human readable pair of IP and port
		Socket sock(AF_INET, SOCK_STREAM);
		sock.Bind("127.0.0.1", 55555);

		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

		sockaddr_in o_address = { 0 };

	#ifdef PLATFORM_WINDOWS
			int length = sizeof(sockaddr_in);
	#else//Unix like platform
			socklen_t length = sizeof(sockaddr_in);
	#endif

		getsockname(sock.GetNativeFD(), (sockaddr*)&o_address, &length);

		REQUIRE(address.sin_port == o_address.sin_port);
	#ifdef PLATFORM_WINDOWS
		REQUIRE(address.sin_addr.S_un.S_addr == o_address.sin_addr.S_un.S_addr);
	#else//Unix like platform
		REQUIRE(address.sin_addr.s_addr == o_address.sin_addr.s_addr);
	#endif
	}
}

TEST_CASE("Testing Listen()", "[Socket]")
{
	Socket sock(AF_INET, SOCK_STREAM);
	sock.Bind();
	sock.Listen();
}

TEST_CASE("Testing Accept()", "Socket")
{
	{//Accept without Address
		auto task = std::async(std::launch::async, []()
		{
			Socket sock(AF_INET, SOCK_STREAM);
			sock.Bind("127.0.0.1", 55555);
			sock.Listen();
			Socket client = sock.Accept();
			REQUIRE(client.GetNativeFD() != INVALID_SOCKET);
			REQUIRE(sock.m_AF == AF_INET);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		Socket sock(AF_INET, SOCK_STREAM);
		
		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

		int result = connect(sock.GetNativeFD(), (sockaddr*)&address, sizeof(sockaddr_in));
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
			Socket sock(AF_INET, SOCK_STREAM);
			sock.Bind("127.0.0.1", 55555);
			sock.Listen();
			sockaddr_in address = { 0 };
			Socket client = sock.Accept(address);

			REQUIRE(client.GetNativeFD() != INVALID_SOCKET);
			REQUIRE(sock.m_AF == AF_INET);

			REQUIRE(clientAddress.sin_port == address.sin_port);
		#ifdef PLATFORM_WINDOWS
			REQUIRE(clientAddress.sin_addr.S_un.S_addr == address.sin_addr.S_un.S_addr);
		#else//Unix like platform
			REQUIRE(clientAddress.sin_addr.s_addr == address.sin_addr.s_addr);
		#endif
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding


		Socket sock(AF_INET, SOCK_STREAM);
		sock.Bind(clientAddress);

		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

		int result = connect(sock.GetNativeFD(), (sockaddr*)&address, sizeof(sockaddr_in));
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
			Socket sock(AF_INET6, SOCK_STREAM);
			sock.Bind("::1", 55555);
			sock.Listen();
			sockaddr_in6 address = { 0 };
			Socket client = sock.Accept(address);

			REQUIRE(client.GetNativeFD() != INVALID_SOCKET);
			REQUIRE(sock.m_AF == AF_INET6);

			REQUIRE(clientAddress.sin6_port == address.sin6_port);
			for (size_t i = 0; i < 16; i++)
			{
			#ifdef PLATFORM_WINDOWS
				REQUIRE(clientAddress.sin6_addr.u.Byte[i] == address.sin6_addr.u.Byte[i]);
			#else//Unix like platforms
				REQUIRE(clientAddress.sin6_addr.s6_addr[i] == address.sin6_addr.s6_addr[i]);
			#endif
			}
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		Socket sock(AF_INET6, SOCK_STREAM);
		sock.Bind(clientAddress);

		sockaddr_in6 address = { 0 };
		address.sin6_family = AF_INET6;
		address.sin6_port = htons(55555);
		inet_pton(AF_INET6, "::1", &address.sin6_addr);

		int result = connect(sock.GetNativeFD(), (sockaddr*)&address, sizeof(sockaddr_in6));
		REQUIRE(result != SOCKET_ERROR);

		task.wait();
	}
}

TEST_CASE("Testing Connect()", "[Socket]")
{
	{//Connect using sockaddr_in
		auto task = std::async(std::launch::async, []()
		{
			Socket sock(AF_INET, SOCK_STREAM);
			sock.Bind("127.0.0.1", 55555);
			sock.Listen();
			Socket client = sock.Accept();
			REQUIRE(client.GetNativeFD() != INVALID_SOCKET);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		Socket client(AF_INET, SOCK_STREAM);

		sockaddr_in address = { 0 };
		address.sin_family = AF_INET;
		address.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
		client.Connect(address);

		task.wait();
	}

	{//Connect using sockaddr_in6
		
		auto task = std::async(std::launch::async, []()
		{
			Socket sock(AF_INET6, SOCK_STREAM);
			sock.Bind("::1", 55555);
			sock.Listen();
			Socket client = sock.Accept();
			REQUIRE(client.GetNativeFD() != INVALID_SOCKET);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		Socket client(AF_INET6, SOCK_STREAM);

		sockaddr_in6 address = { 0 };
		address.sin6_family = AF_INET6;
		address.sin6_port = htons(55555);
		inet_pton(AF_INET6, "::1", &address.sin6_addr);

		client.Connect(address);

		task.wait();
	}

	{//Connect using the python way (IPv4)
		auto task = std::async(std::launch::async, []()
		{
			Socket sock(AF_INET, SOCK_STREAM);
			sock.Bind("127.0.0.1", 55555);
			sock.Listen();
			Socket client = sock.Accept();
			REQUIRE(client.GetNativeFD() != INVALID_SOCKET);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		Socket client(AF_INET, SOCK_STREAM);
		client.Connect("127.0.0.1", 55555);

		task.wait();
	}

	{//Connect using the python way (IPv6)
		auto task = std::async(std::launch::async, []()
		{
			Socket sock(AF_INET6, SOCK_STREAM);
			sock.Bind("::1", 55555);
			sock.Listen();
			Socket client = sock.Accept();
			REQUIRE(client.GetNativeFD() != INVALID_SOCKET);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		Socket client(AF_INET6, SOCK_STREAM);
		client.Connect("::1", 55555);

		task.wait();
	}
}

TEST_CASE("Testing Stream Transmition", "[Socket]")
{

	{//IPv4
		auto task = std::async(std::launch::async, []()
		{
			Socket server(AF_INET, SOCK_STREAM);
			server.Bind("127.0.0.1", 55555);
			server.Listen();
			Socket client = server.Accept();

			
			char buffer[12];
			
			int bytes = client.Receive(buffer, 6);
			std::string expected = "Hello";
			REQUIRE(bytes == expected.size()+1);
			REQUIRE(expected.compare(buffer) == 0);

			bytes = client.Receive(buffer, 6, 6);
			expected = "World";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected.compare(&buffer[6]));
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		constexpr char str[12] = "Hello\0Wolrd";

		Socket client(AF_INET, SOCK_STREAM);
		client.Connect("127.0.0.1", 55555);
		
		int bytes = client.Send(str, 6);
		REQUIRE(bytes == 6);
		bytes = client.Send(str, 6, 6);
		REQUIRE(bytes == 6);

		task.wait();
	}

	{//IPv6
		auto task = std::async(std::launch::async, []()
		{
			Socket server(AF_INET6, SOCK_STREAM);
			server.Bind("::1", 55555);
			server.Listen();
			Socket client = server.Accept();


			char buffer[12];

			int bytes = client.Receive(buffer, 6);
			std::string expected = "Hello";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected.compare(buffer) == 0);

			bytes = client.Receive(buffer, 6, 6);
			expected = "World";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected.compare(&buffer[6]));
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		constexpr char str[12] = "Hello\0Wolrd";

		Socket client(AF_INET6, SOCK_STREAM);
		client.Connect("::1", 55555);

		int bytes = client.Send(str, 6);
		REQUIRE(bytes == 6);
		bytes = client.Send(str, 6, 6);
		REQUIRE(bytes == 6);

		task.wait();
	}
}

TEST_CASE("Testing Datagram Transmition", "[Socket]")
{

	{//IPv4
		sockaddr_in senderAddress = { 0 };
		senderAddress.sin_family = AF_INET;
		senderAddress.sin_port = htons(55556);
		inet_pton(AF_INET, "127.0.0.1", &senderAddress.sin_addr);

		auto task = std::async(std::launch::async, [=]()
		{
			Socket sock(AF_INET, SOCK_DGRAM);
			sock.Bind("127.0.0.1", 55555);

			char buffer[12];

			sockaddr_in address = { 0 };
			int bytes = sock.ReceiveFrom(buffer, address, 6);
			std::string expected = "Hello";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected.compare(buffer) == 0);
			REQUIRE(address.sin_port == senderAddress.sin_port);
		#ifdef PLATFORM_WINDOWS
			REQUIRE(address.sin_addr.S_un.S_addr == senderAddress.sin_addr.S_un.S_addr);
		#else//Unix like platform
			REQUIRE(address.sin_addr.s_addr == senderAddress.sin_addr.s_addr);
		#endif

			address = { 0 };
			bytes = sock.ReceiveFrom(buffer, address, 6, 6);
			expected = "World";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected.compare(&buffer[6]));
			REQUIRE(address.sin_port == senderAddress.sin_port);
		#ifdef PLATFORM_WINDOWS
			REQUIRE(address.sin_addr.S_un.S_addr == senderAddress.sin_addr.S_un.S_addr);
		#else//Unix like platform
			REQUIRE(address.sin_addr.s_addr == senderAddress.sin_addr.s_addr);
		#endif
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		constexpr char str[12] = "Hello\0Wolrd";

		Socket sock(AF_INET, SOCK_DGRAM);
		sock.Bind(senderAddress);

		sockaddr_in recverAddress = { 0 };
		recverAddress.sin_family = AF_INET;
		recverAddress.sin_port = htons(55555);
		inet_pton(AF_INET, "127.0.0.1", &recverAddress.sin_addr);
		
		int bytes = sock.SendTo(str, recverAddress, 6);
		REQUIRE(bytes == 6);
		bytes = sock.SendTo(str, recverAddress, 6, 6);
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
			Socket sock(AF_INET6, SOCK_DGRAM);
			sock.Bind("::1", 55555);

			char buffer[12];

			sockaddr_in6 address = { 0 };
			int bytes = sock.ReceiveFrom(buffer, address, 6);
			std::string expected = "Hello";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected.compare(buffer) == 0);
			REQUIRE(address.sin6_port == senderAddress.sin6_port);
			for (size_t i = 0; i < 16; i++)
			{
			#ifdef PLATFORM_WINDOWS
				REQUIRE(address.sin6_addr.u.Byte[i] == senderAddress.sin6_addr.u.Byte[i]);
			#else//Unix like platforms
				REQUIRE(address.sin6_addr.s6_addr[i] == senderAddress.sin6_addr.s6_addr[i]);
			#endif
			}

			address = { 0 };
			bytes = sock.ReceiveFrom(buffer, address, 6, 6);
			expected = "World";
			REQUIRE(bytes == expected.size() + 1);
			REQUIRE(expected.compare(&buffer[6]));
			REQUIRE(address.sin6_port == senderAddress.sin6_port);
			for (size_t i = 0; i < 16; i++)
			{
			#ifdef PLATFORM_WINDOWS
				REQUIRE(address.sin6_addr.u.Byte[i] == senderAddress.sin6_addr.u.Byte[i]);
			#else//Unix like platforms
				REQUIRE(address.sin6_addr.s6_addr[i] == senderAddress.sin6_addr.s6_addr[i]);
			#endif
			}
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));//Make sure the task starts before procceding

		constexpr char str[12] = "Hello\0Wolrd";

		Socket sock(AF_INET6, SOCK_DGRAM);
		sock.Bind(senderAddress);

		sockaddr_in6 recverAddress = { 0 };
		recverAddress.sin6_family = AF_INET6;
		recverAddress.sin6_port = htons(55555);
		inet_pton(AF_INET6, "::1", &recverAddress.sin6_addr);

		int bytes = sock.SendTo(str, recverAddress, 6);
		REQUIRE(bytes == 6);
		bytes = sock.SendTo(str, recverAddress, 6, 6);
		REQUIRE(bytes == 6);

		task.wait();
	}
}

TEST_CASE("Testing Copy assignement operator", "[Socket]")
{
	Socket sock;
	Socket copy; copy = sock;
	REQUIRE(sock.GetNativeFD() == copy.GetNativeFD());
	REQUIRE(sock.m_AF == copy.m_AF);
	REQUIRE(sock.m_BlockMode == copy.m_BlockMode);
	REQUIRE(sock.m_SockRef.use_count() == 2);
}

TEST_CASE("Testing Move assignement operator", "[Socket]")
{
	{
		Socket sock(AF_INET, SOCK_STREAM);
		Socket moved; moved=std::move(sock);

		REQUIRE(sock.GetNativeFD() == INVALID_SOCKET);
		REQUIRE(sock.GetNativeFD() != moved.GetNativeFD());
		REQUIRE(sock.m_AF != moved.m_AF);
		REQUIRE(sock.m_AF == AF_UNSPEC);
		REQUIRE(sock.m_BlockMode == moved.m_BlockMode);
		REQUIRE(sock.m_SockRef.use_count() == 1);
		REQUIRE(moved.m_SockRef.use_count() == 1);
	}

	{
		Socket sock(AF_INET, SOCK_STREAM);
		Socket copy(sock);
		Socket moved; moved = std::move(sock);

		REQUIRE(sock.GetNativeFD() == INVALID_SOCKET);
		REQUIRE(sock.GetNativeFD() != moved.GetNativeFD());
		REQUIRE(copy.GetNativeFD() == moved.GetNativeFD());
		REQUIRE(sock.m_AF != moved.m_AF);
		REQUIRE(moved.m_AF == copy.m_AF);
		REQUIRE(sock.m_AF == AF_UNSPEC);
		REQUIRE(sock.m_BlockMode == moved.m_BlockMode);
		REQUIRE(sock.m_SockRef.use_count() == 1);
		REQUIRE(moved.m_SockRef.use_count() == 2);
	}
}