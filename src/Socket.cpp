#include <socklib/Socket.h>
#include <cstring>

//Declaration of helper functions
void CreateAddress(const char* address, unsigned short port, sockaddr_in& sockAddress) noexcept;
void CreateAddress(const char* address, unsigned short port, sockaddr_in6& sockAddress) noexcept;
std::string GetError() noexcept;
bool HasTimeoutError() noexcept;
//End Declaration of helper functions

namespace socklib {

	void Socket::Open(const AddressFamily family, const SocketType type, const int proto) noexcept
	{
		if(mSockRef.use_count() == 0)
			mSockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
		SOCKET& sock = *mSockRef;
		SOCKLIB_ASSERT(sock == INVALID_SOCKET, "Socket is already opened!");

		sock = socket(static_cast<int>(family), static_cast<int>(type), proto);
		SOCKLIB_ASSERT(sock != INVALID_SOCKET, GetError().c_str());

#ifdef PLATFORM_WINDOWS
		constexpr BOOL val = TRUE;
		const int result = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(BOOL));
#else
		constexpr int val = 1;
		const int result = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
#endif
		SOCKLIB_ASSERT(result != SOCKET_ERROR, "Failed to set SO_REUSEADDR!");

		mAF = family;
	}

	void Socket::Bind(const char* address, const unsigned short port) const noexcept
	{
		switch (mAF)
		{
		case AddressFamily::IPv4:
		{
			sockaddr_in sock_address = { 0 };
			CreateAddress(address, port, sock_address);
			Bind(reinterpret_cast<sockaddr*>(&sock_address), sizeof(sockaddr_in));
			break;
		}
		case AddressFamily::IPv6:
		{
			sockaddr_in6 sock_address = { 0 };
			CreateAddress(address, port, sock_address);
			Bind(reinterpret_cast<sockaddr*>(&sock_address), sizeof(sockaddr_in6));
			break;
		}
		default:
			SOCKLIB_ASSERT(false, "Currently only IPv4 and IPv6 is supported!");
			break;
		}
	}

	void Socket::Bind(const sockaddr* address, const socklen_t size) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(mAF == static_cast<AddressFamily>(address->sa_family), "Socket hasn't opened with same address Family!");
		const int result = bind(*mSockRef, address, size);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	}

	void Socket::Bind(const std::string& address, const unsigned short port) const noexcept { Bind(address.c_str(), port);  }

	void Socket::Shutdown(const int how) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed!");
		const int result = shutdown(*mSockRef, how);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	}

	void Socket::Close() noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed!");
		const int result = closesocket(*mSockRef);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
		*mSockRef = INVALID_SOCKET;
		mAF = AddressFamily::UNSPECIFIED;
	}

	void Socket::Connect(const Endpoint &endpoint) const noexcept
	{
		Connect(endpoint.Host.c_str(), endpoint.Port);
	}

	void Socket::Connect(const char* address, const unsigned short port) const noexcept
	{
		switch (mAF)
		{
		case AddressFamily::IPv4:
		{
			sockaddr_in sock_address = { 0 };
			CreateAddress(address, port, sock_address);
			Connect(reinterpret_cast<sockaddr*>(&sock_address), sizeof(sockaddr_in));
			break;
		}
		case AddressFamily::IPv6:
		{
			sockaddr_in6 sock_address = { 0 };
			CreateAddress(address, port, sock_address);
			Connect(reinterpret_cast<sockaddr*>(&sock_address), sizeof(sockaddr_in6));
			break;
		}
		default:
			SOCKLIB_ASSERT(false, "Currently only IPv4 and IPv6 is supported!");
			break;
		}
	}

	void Socket::Connect(const sockaddr* address, const socklen_t size) const noexcept
	{
		SOCKLIB_ASSERT(mAF == static_cast<AddressFamily>(address->sa_family), "Socket hasn't opened with same address Family!");
		const int result = connect(*mSockRef, address, size);
		if (HasTimeoutError()) return;
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	}

	void Socket::Connect(const std::string& address, const unsigned short port) const noexcept { Connect(address.c_str(), port); }

	void Socket::Listen(const int length) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		const int result = listen(*mSockRef, length);
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	}

	std::pair<Socket, Endpoint> Socket::Accept() const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		Socket clientSock{};
		
		std::string ip;
		unsigned short port = 0;
		switch (mAF)
		{
		case AddressFamily::IPv4:
		{
			char buffer[16] = { 0 };
			sockaddr_in address = { 0 };
			socklen_t size = sizeof(sockaddr_in);
			clientSock = Accept(reinterpret_cast<sockaddr*>(&address), &size);
			if (HasTimeoutError()) return std::make_pair<Socket, Endpoint>({}, {});
			inet_ntop(AF_INET, &address.sin_addr, buffer, 16);
			ip = std::string(buffer);
			port = ntohs(address.sin_port);
			break;
		}
		case AddressFamily::IPv6:
		{
			char buffer[46] = { 0 };
			sockaddr_in6 address = { 0 };
			socklen_t size = sizeof(sockaddr_in6);
			clientSock = Accept(reinterpret_cast<sockaddr*>(&address), &size);

			if (HasTimeoutError()) return std::make_pair<Socket, Endpoint>({}, {});
			inet_ntop(AF_INET6, &address.sin6_addr, buffer, 46);
			ip = std::string(buffer);
			port = ntohs(address.sin6_port);
			break;
		}
		default:
			break;
		}

		clientSock.mAF = mAF;
		return std::make_pair(clientSock, Endpoint{ ip, port });
	}

	Socket Socket::Accept(sockaddr* address, socklen_t* size) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");

		Socket client{};
		client.mSockRef = std::make_shared<SOCKET>(accept(*mSockRef, address, size));

		if (HasTimeoutError()) return{};
		SOCKLIB_ASSERT(*client.mSockRef != INVALID_SOCKET, GetError().c_str());
		client.mAF = mAF;

		return client;
	}


	IOSize Socket::Send(const void* data, const size_t length, const size_t offset) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		const char* buffer = static_cast<const char*>(data) + offset;

		const IOSize bytes = send(*mSockRef, buffer, length, 0);

		if (HasTimeoutError()) return -1;
		SOCKLIB_ASSERT(bytes != -1, GetError().c_str());
		return bytes;
	}

	IOSize Socket::SendTo(const void* data, const sockaddr* address, const socklen_t addressSize, const size_t length, const size_t offset) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		SOCKLIB_ASSERT(mAF == static_cast<AddressFamily>(address->sa_family), "Socket hasn't opened with same address Family!");
		const char* buffer = static_cast<const char *>(data) + offset;

		const IOSize bytes = sendto(*mSockRef, buffer, length, 0, address, addressSize);

		SOCKLIB_ASSERT(bytes != -1, GetError().c_str());
		return bytes;
	}

	IOSize Socket::SendTo(const void* data, const Endpoint& endpoint, const size_t length, const size_t offset) const noexcept
	{
		switch (mAF)
		{
		case AddressFamily::IPv4:
		{
			sockaddr_in address = { 0 };
			CreateAddress(endpoint.Host.c_str(), endpoint.Port, address);
			return SendTo(data, reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in), length, offset);
		}
		case AddressFamily::IPv6:
		{
			sockaddr_in6 address = { 0 };
			CreateAddress(endpoint.Host.c_str(), endpoint.Port, address);
			return SendTo(data, reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_in6), length, offset);
		}
		default:
			SOCKLIB_ASSERT(false, "Not supported address family!");
			return SOCKET_ERROR;//Not suppose to be reached
		}
	}

	IOSize Socket::Receive(void* data, const size_t length, const size_t offset) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		char* buffer = static_cast<char*>(data) + offset;

		const IOSize bytes = recv(*mSockRef, buffer, length, 0);

		if (HasTimeoutError()) return -1;
		SOCKLIB_ASSERT(bytes != -1, GetError().c_str());
		return bytes;
	}

	IOSize Socket::ReceiveFrom(void* data, sockaddr* address, socklen_t* addressSize, const size_t length, const size_t offset) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");

		char* buffer = static_cast<char *>(data) + offset;

		const IOSize bytes = recvfrom(*mSockRef, buffer, length, 0, address, addressSize);

		if (bytes == -1 && HasTimeoutError()) return -1;
		SOCKLIB_ASSERT(bytes != -1, GetError().c_str());
		return bytes;
	}

	std::pair<IOSize, Endpoint> Socket::ReceiveFrom(void* data, const size_t length, const size_t offset) const noexcept
	{
		std::string ip;
		unsigned short port = 0;
		IOSize bytes = SOCKET_ERROR;
		switch (mAF)
		{
			case AddressFamily::IPv4:
			{
				char buffer[16] = { 0 };
				sockaddr_in address = { 0 };
				socklen_t addressSize = sizeof(sockaddr_in);
				bytes = ReceiveFrom(data, reinterpret_cast<sockaddr*>(&address), &addressSize, length, offset);

				if (bytes == -1 && HasTimeoutError()) return std::make_pair(-1, Endpoint{});
				inet_ntop(AF_INET, &address.sin_addr, buffer, 16);
				ip = std::string(buffer);
				port = ntohs(address.sin_port);
				break;
			}
			case AddressFamily::IPv6:
			{
				char buffer[46] = { 0 };
				sockaddr_in6 address = { 0 };
				socklen_t addressSize = sizeof(sockaddr_in6);
				bytes = ReceiveFrom(data, reinterpret_cast<sockaddr*>(&address), &addressSize, length, offset);

				if (bytes == -1 && HasTimeoutError()) return std::make_pair(-1, Endpoint{});
				SOCKLIB_ASSERT(bytes != -1, GetError().c_str());
				inet_ntop(AF_INET6, &address.sin6_addr, buffer, 46);
				ip = std::string(buffer);
				port = ntohs(address.sin6_port);
				break;
			}
			default:
				SOCKLIB_ASSERT(false, "Not supported address family!");
			return std::make_pair(SOCKET_ERROR, Endpoint {"", 0 });//Not suppose to be reached
		}
		return std::make_pair(bytes, Endpoint{ ip, port });
	}

	Socket::Socket(Socket&& other) noexcept
	{
		mAF = other.mAF;
		mBlockMode = other.mBlockMode;

		mSockRef = std::move(other.mSockRef);
		other.mAF = AddressFamily::UNSPECIFIED;
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
		rhs.mAF = AddressFamily::UNSPECIFIED;
		rhs.mBlockMode = true;
		return *this;
	}

	Socket::~Socket() noexcept
	{
		if ((mSockRef.use_count() == 1) && (*mSockRef != INVALID_SOCKET))
			Close();
	}

	Socket Socket::CreateConnection(const AddressFamily family, const Endpoint& endpoint, const uint32_t timeout, const Endpoint& local) noexcept
	{
		Socket client(family, SocketType::STREAM, IPPROTO_TCP);
		client.Bind(local.Host, local.Port);
		if(timeout > 0)
			client.SetTimeout(timeout);
		client.Connect(endpoint.Host, endpoint.Port);
		return client;
	}

	// ************************************************************************
	// | Bellow from here the implementation differ depending on the Platform |
	// ************************************************************************

	Socket Socket::CreateServer(const AddressFamily family, const Endpoint& endpoint, const int queue) noexcept
	{
		Socket server(family, SocketType::STREAM, IPPROTO_TCP);
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

	void Socket::SetTimeout(const uint32_t millis) const noexcept
	{
		SOCKLIB_ASSERT(mSockRef.use_count() >= 1, "Socket is not opened!");
		SOCKLIB_ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed");
		int result;
	#ifdef PLATFORM_WINDOWS
		result = setsockopt(*mSockRef, SOL_SOCKET, SO_SNDTIMEO, (char*)&millis, sizeof(uint32_t));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
		result = setsockopt(*mSockRef, SOL_SOCKET, SO_RCVTIMEO, (char*)&millis, sizeof(uint32_t));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	#else
		timeval timeout{};
		timeout.tv_sec = static_cast<long>(millis) / 1000;
		timeout.tv_usec = (static_cast<long>(millis) % 1000)* 1000;
		result = setsockopt(*mSockRef, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeval));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
		result = setsockopt(*mSockRef, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval));
		SOCKLIB_ASSERT(result != SOCKET_ERROR, GetError().c_str());
	#endif
	}

	SOCKET Socket::FileNo() const noexcept { return mSockRef.use_count() == 0 ? INVALID_SOCKET : *mSockRef; }

}

// ********************
// | Helper functions |
// ********************

void CreateAddress(const char* address, const unsigned short port, sockaddr_in& sockAddress) noexcept
{
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_port = htons(port);
	if (address != nullptr)
		inet_pton(AF_INET, address, &sockAddress.sin_addr);
	else
		sockAddress.sin_addr.s_addr = INADDR_ANY;
}

void CreateAddress(const char* address, const unsigned short port, sockaddr_in6& sockAddress) noexcept
{
	sockAddress.sin6_family = AF_INET6;
	sockAddress.sin6_port = htons(port);
	if (address != nullptr)
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

	bool HasTimeoutError() noexcept
	{
		const int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK || err == WSAETIMEDOUT)
			return true;
		WSASetLastError(err);
		return false;
	}
#else//Unix like platforms
	std::string GetError() noexcept
	{
		const int err = errno;
		return { strerror(err) };
	}

	bool HasTimeoutError() noexcept
	{
		const int err = errno;
		if (err == EAGAIN || err == ETIMEDOUT)
			return true;
		errno = err;
		return false;
	}
#endif