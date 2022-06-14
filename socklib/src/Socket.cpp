#include <socklib/Socket.h>
#include <cstring>

//TODO(Vasilis): Think if we need to assert at Accept and IO (I'm curious about what I should do on timeout)

//Declaration of helper functions
void CreateAddress(const char* address, unsigned short port, sockaddr_in& sockAddress) noexcept;
void CreateAddress(const char* address, unsigned short port, sockaddr_in6& sockAddress) noexcept;
std::string GetError(void) noexcept;
//End Declaration of helper functions

namespace socklib {

	void Socket::Open(int family, int type, int proto) noexcept
	{
		if(mSockRef.use_count() == 0)
			mSockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
		SOCKET& sock = *mSockRef;
		SOCKLIB_ASSERT(sock == INVALID_SOCKET, "Socket is already opened!");
		sock = socket(family, type, proto);
		SOCKLIB_ASSERT(sock != INVALID_SOCKET, GetError().c_str());
		mAF = family;
	}

	void Socket::Bind(const char* address, unsigned short port) const noexcept
	{
		switch (mAF)
		{
		case AF_INET:
		{
			sockaddr_in sock_address = { 0 };
			CreateAddress(address, port, sock_address);
			Bind((sockaddr*)&sock_address, sizeof(sockaddr_in));
			break;
		}
		case AF_INET6:
		{
			sockaddr_in6 sock_address = { 0 };
			CreateAddress(address, port, sock_address);
			Bind((sockaddr*)&sock_address, sizeof(sockaddr_in6));
			break;
		}
		default:
			SOCKLIB_ASSERT(false, "Currently only IPv4 and IPv6 is supported!");
			break;
		}
	}

	void Socket::Bind(const sockaddr* address, socklen_t size) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(mAF == address->sa_family, "Socket hasn't opened with same address Family!");
		int result = bind(*mSockRef, address, size);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	}

	void Socket::Bind(const std::string& address, unsigned short port) const noexcept { Bind(address.c_str(), port);  }

	void Socket::Shutdown(int how) noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed!");
		int result = shutdown(*mSockRef, how);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	}

	void Socket::Close(void) noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed!");
		int result = closesocket(*mSockRef);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
		*mSockRef = INVALID_SOCKET;
		mAF = AF_UNSPEC;
	}

	void Socket::Connect(const char* address, unsigned short port) const noexcept
	{
		switch (mAF)
		{
		case AF_INET:
		{
			sockaddr_in sock_address = { 0 };
			CreateAddress(address, port, sock_address);
			Connect((sockaddr*)&sock_address, sizeof(sockaddr_in));
			break;
		}
		case AF_INET6:
		{
			sockaddr_in6 sock_address = { 0 };
			CreateAddress(address, port, sock_address);
			Connect((sockaddr*)&sock_address, sizeof(sockaddr_in6));
			break;
		}
		default:
			SOCKLIB_ASSERT(false, "Currently only IPv4 and IPv6 is supported!");
			break;
		}
	}

	void Socket::Connect(const sockaddr* address, socklen_t size) const noexcept
	{
		SOCKLIB_ASSERT(mAF == address->sa_family, "Socket hasn't opened with same address Family!");
		int result = connect(*mSockRef, address, size);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	}

	void Socket::Connect(const std::string& address, unsigned short port) const noexcept { Connect(address.c_str(), port); }

	void Socket::Listen(int length) noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		int result = listen(*mSockRef, length);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	}

	std::pair<Socket, std::pair<std::string, unsigned short>> Socket::Accept(void) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		Socket clientSock{};
		
		std::string ip;
		unsigned short port = 0;
		switch (mAF)
		{
		case AF_INET:
		{
			char buffer[16];
			memset(buffer, 0, 16);
			sockaddr_in address = { 0 };
			socklen_t size = sizeof(sockaddr_in);
			clientSock = Accept((sockaddr*)&address, &size);
			inet_ntop(AF_INET, &address.sin_addr, buffer, 16);
			ip = std::string(buffer);
			port = ntohs(address.sin_port);
			break;
		}
		case AF_INET6:
		{
			char buffer[46];
			memset(buffer, 0, 46);
			sockaddr_in6 address = { 0 };
			socklen_t size = sizeof(sockaddr_in6);
			clientSock = Accept((sockaddr*)&address, &size);
			inet_ntop(AF_INET6, &address.sin6_addr, buffer, 46);
			ip = std::string(buffer);
			port = ntohs(address.sin6_port);
			break;
		}
		default:
			break;
		}

		clientSock.mAF = mAF;
		return std::make_pair(clientSock, std::make_pair(ip, port));
	}

	Socket Socket::Accept(sockaddr* address, socklen_t* size) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");

		Socket client{};
		client.mSockRef = std::make_shared<SOCKET>(accept(*mSockRef, address, size));
		//SOCKLIB_ASSERT(*client.mSockRef != INVALID_SOCKET, GetError().c_str());
		client.mAF = mAF;
		return client;
	}


	int Socket::Send(const void* data, size_t length, size_t offset) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		char* buffer = (char*)data + offset;

		int bytes = send(*mSockRef, buffer, length, 0);

		//SOCKLIB_ASSERT(bytes > -1, GetError().c_str());
		return bytes;
	}

	int Socket::SendTo(const void* data, const sockaddr* address, socklen_t addressSize, size_t length, size_t offset) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		SOCKLIB_ASSERT(mAF == address->sa_family, "Socket hasn't opened with same address Family!");
		char* buffer = (char*)data + offset;

		int bytes = sendto(*mSockRef, buffer, length, 0, address, addressSize);

		//SOCKLIB_ASSERT(bytes > -1, GetError().c_str());
		return bytes;
	}

	int Socket::SendTo(const void* data, const Endpoint& endpoint, size_t length, size_t offset) const noexcept
	{
		switch (mAF)
		{
		case AF_INET:
		{
			sockaddr_in address = { 0 };
			CreateAddress(endpoint.Host, endpoint.Port, address);
			return SendTo(data, (sockaddr*)&address, sizeof(sockaddr_in), length, offset);
		}
		case AF_INET6:
		{
			sockaddr_in6 address = { 0 };
			CreateAddress(endpoint.Host, endpoint.Port, address);
			return SendTo(data, (sockaddr*)&address, sizeof(sockaddr_in6), length, offset);
		}
		default:
			SOCKLIB_ASSERT(false, "Not supported address family!");
			return SOCKET_ERROR;//Not suppose to be reached
		}
	}

	int Socket::Receive(void* data, size_t length, size_t offset) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		char* buffer = (char*)data + offset;

		int bytes = recv(*mSockRef, buffer, length, 0);

		//SOCKLIB_ASSERT(bytes > -1, GetError().c_str());
		return bytes;
	}

	int Socket::ReceiveFrom(void* data, sockaddr* address, socklen_t* addressSize, size_t length, size_t offset) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");

		char* buffer = (char*)data + offset;

		int bytes = recvfrom(*mSockRef, buffer, length, 0, address, addressSize);
		//SOCKLIB_ASSERT(bytes > -1, GetError().c_str());
		return bytes;
	}

	std::pair<int, std::pair<std::string, unsigned short>> Socket::ReceiveFrom(void* data, size_t length, size_t offset) const noexcept
	{
		//TODO(Vasilis): Depending on if we have assertion in low level function or not we may
		//	have to check bytes return value
		std::string ip;
		unsigned short port = 0;
		int bytes = SOCKET_ERROR;
		switch (mAF)
		{
		case AF_INET:
		{
			char buffer[16];
			memset(buffer, 0, 16);
			sockaddr_in address = { 0 };
			socklen_t addressSize = sizeof(sockaddr_in);
			bytes = ReceiveFrom(data, (sockaddr*)&address, &addressSize, length, offset);
			inet_ntop(AF_INET, &address.sin_addr, buffer, 16);
			ip = std::string(buffer);
			port = ntohs(address.sin_port);
			break;
		}
		case AF_INET6:
		{
			char buffer[46];
			memset(buffer, 0, 46);
			sockaddr_in6 address = { 0 };
			socklen_t addressSize = sizeof(sockaddr_in6);
			bytes = ReceiveFrom(data, (sockaddr*)&address, &addressSize, length, offset);
			inet_ntop(AF_INET6, &address.sin6_addr, buffer, 46);
			ip = std::string(buffer);
			port = ntohs(address.sin6_port);
			break;
		}
		default:
			SOCKLIB_ASSERT(false, "Not supported address family!");
			return std::make_pair(SOCKET_ERROR, std::make_pair("", 0));//Not suppose to be reached
		}
		return std::make_pair(bytes, std::make_pair(ip, port));
	}

	Socket::Socket(Socket&& other) noexcept
	{
		mAF = other.mAF;
		mBlockMode = other.mBlockMode;

		mSockRef = std::move(other.mSockRef);
		other.mAF = AF_UNSPEC;
		other.mBlockMode = true;
	}

	Socket& Socket::operator=(Socket&& rhs) noexcept
	{
		if (this == &rhs) return *this;

		if ((mSockRef.use_count() == 1) && (*mSockRef != INVALID_SOCKET))
			Close();

		mAF = rhs.mAF;
		mBlockMode = rhs.mBlockMode;

		mSockRef = std::move(rhs.mSockRef);
		rhs.mAF = AF_UNSPEC;
		rhs.mBlockMode = true;
		return *this;
	}

	Socket::~Socket(void) noexcept
	{
		if ((mSockRef.use_count() == 1) && (*mSockRef != INVALID_SOCKET))
			Close();
	}

	Socket Socket::CreateConnection(int family, const Endpoint& endpoint, uint64_t timeout, const Endpoint& local) noexcept
	{
		Socket client(family, SOCK_STREAM, IPPROTO_TCP);
		client.Bind(local.Host, local.Port);
		if(timeout > 0)
			client.SetTimeout(timeout);
		client.Connect(endpoint.Host, endpoint.Port);
		return client;
	}

	// ************************************************************************
	// | Bellow from here the implementation differ depending on the Platform |
	// ************************************************************************

	Socket Socket::CreateServer(int family, const Endpoint& endpoint, int queue) noexcept
	{
		Socket server(family, SOCK_STREAM, IPPROTO_TCP);
	#ifndef PLATFORM_WINDOWS
		int val = 1;
		int result = setsockopt(server.Fileno(), SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, "Failed to set SO_REUSEADDR!");
	#endif
		server.Bind(endpoint.Host, endpoint.Port);
		server.Listen(queue);
		return server;
	}

	void Socket::SetBlocking(bool flag) noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		if (flag == mBlockMode) return;
		int result = SOCKET_ERROR;
	#ifdef PLATFORM_WINDOWS
		unsigned long iMode = (unsigned long)!flag;
		result = ioctlsocket(*mSockRef, FIONBIO, &iMode);
	#else
		int flags = fcntl(*mSockRef, F_GETFL);
		flags = !flag ? (flags | O_NONBLOCK) : (flags & (~O_NONBLOCK));
		result = ioctl(*mSockRef, F_SETFL, flags);
	#endif
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
		mBlockMode = flag;
	}

	void Socket::SetTimeout(uint64_t milis) noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		int result;
	#ifdef PLATFORM_WINDOWS
		result = setsockopt(*mSockRef, SOL_SOCKET, SO_SNDTIMEO, (char*)&milis, sizeof(uint64_t));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
		result = setsockopt(*mSockRef, SOL_SOCKET, SO_RCVTIMEO, (char*)&milis, sizeof(uint64_t));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	#else
		timeval timeout;
		timeout.tv_usec = milis * 1000.0f;
		result = setsockopt(*mSockRef, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeval));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
		result = setsockopt(*mSockRef, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeval));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	#endif
	}

	SOCKET Socket::Fileno(void) const noexcept { return mSockRef.use_count() == 0 ? INVALID_SOCKET : *mSockRef; }

}

// ********************
// | Helper functions |
// ********************

void CreateAddress(const char* address, unsigned short port, sockaddr_in& sockAddress) noexcept
{
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_port = htons(port);
	if (address != NULL)
		inet_pton(AF_INET, address, &sockAddress.sin_addr);
	else
		sockAddress.sin_addr.s_addr = INADDR_ANY;
}

void CreateAddress(const char* address, unsigned short port, sockaddr_in6& sockAddress) noexcept
{
	sockAddress.sin6_family = AF_INET6;
	sockAddress.sin6_port = htons(port);
	if (address != NULL)
		inet_pton(AF_INET6, address, &sockAddress.sin6_addr);
	else
		sockAddress.sin6_addr = in6addr_any;
}

#ifdef PLATFORM_WINDOWS
	std::string GetError(void) noexcept
	{
		std::string rdata;
		char* buffer;
		int error = WSAGetLastError();
		if (FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPSTR)&buffer, 0, NULL
		))
		{
			rdata = std::string(buffer);
			LocalFree(buffer);
		}
		else
			rdata = std::string("Unknown error!");
		return rdata;
	}
#else//Unix like platforms
	#include <string.h>
	std::string GetError(void) noexcept
	{
		return std::string(strerror(errno));
	}
#endif