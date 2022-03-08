#include <socklib/socket.h>
#include <string>

//Declaration of helper functions
void CreateAddress(const char* address, unsigned short port, sockaddr_in& sockAddress) noexcept;
void CreateAddress(const char* address, unsigned short port, sockaddr_in6& sockAddress) noexcept;
std::string GetError(void) noexcept;
//End Declaration of helper functions


Socket::Socket(void) noexcept
{
	mSockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
}

void Socket::Open(int family, int type, int proto) noexcept
{
	SOCKET& sock = *mSockRef;
	ASSERT(sock == INVALID_SOCKET, "Socket is already opened!");
	sock = socket(family, type, proto);
	ASSERT(sock != INVALID_SOCKET, GetError().c_str());
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
		ASSERT(false, "Currently only IPv4 and IPv6 is supported!");
		break;
	}
}

void Socket::Bind(const sockaddr* address, socklen_t size) const noexcept
{
	ASSERT(mAF == address->sa_family, "Socket hasn't opened with same address Family!");
	int result = bind(*mSockRef, address, size);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
}

void Socket::Bind(const std::string& address, unsigned short port) const noexcept { Bind(address.c_str(), port);  }

void Socket::Shutdown(int how) noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is not opened!")
	int result = shutdown(*mSockRef, how);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
}

void Socket::Close(void) noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is already closed!");
	int result = closesocket(*mSockRef);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
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
		ASSERT(false, "Currently only IPv4 and IPv6 is supported!");
		break;
	}
}

void Socket::Connect(const sockaddr* address, socklen_t size) const noexcept
{
	ASSERT(mAF == address->sa_family, "Socket hasn't opened with same address Family!");
	int result = connect(*mSockRef, address, size);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
}

void Socket::Connect(const std::string& address, unsigned short port) const noexcept { Connect(address.c_str(), port); }

void Socket::Listen(int length) noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is not opened!");
	int result = listen(*mSockRef, length);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
}

std::pair<Socket, std::pair<std::string, unsigned short>> Socket::Accept(void) const noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is not opened!");
	Socket clientSock{};
	
	std::string ip;
	unsigned short port = 0;
	switch (mAF)
	{
	case AF_INET:
	{
		char buffer[16];
		sockaddr_in address = { 0 };
		socklen_t size = sizeof(sockaddr_in);
		*clientSock.mSockRef = accept(*mSockRef, (sockaddr*)&address, &size);
		ASSERT(*clientSock.mSockRef != INVALID_SOCKET, GetError().c_str());
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
		*clientSock.mSockRef = accept(*mSockRef, (sockaddr*)&address, &size);
		ASSERT(*clientSock.mSockRef != INVALID_SOCKET, GetError().c_str());
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
	ASSERT(*mSockRef != INVALID_SOCKET, "The Socket is not opened!");

	Socket client{};
	*client.mSockRef = accept(*mSockRef, address, size);
	ASSERT(*client.mSockRef != INVALID_SOCKET, GetError().c_str());
	client.mAF = mAF;
	return client;
}


int Socket::Send(const void* data, unsigned int length, unsigned int offset) const noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "Socket is not opened!");
	char* buffer = (char*)data + offset;

	int bytes = send(*mSockRef, buffer, length, 0);

	ASSERT(bytes > -1, GetError().c_str());
	return bytes;
}

int Socket::SendTo(const void* data, const sockaddr* address, socklen_t addressSize, unsigned int length, unsigned int offset) const noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "Socket is not opened!");
	ASSERT(mAF == address->sa_family, "Socket hasn't opened with same address Family!");
	char* buffer = (char*)data + offset;

	int bytes = sendto(*mSockRef, buffer, length, 0, address, addressSize);

	ASSERT(bytes > -1, GetError().c_str());
	return bytes;
}

int Socket::Receive(void* data, unsigned int length, unsigned int offset) const noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "Socket is not opened!");
	char* buffer = (char*)data + offset;

	int bytes = recv(*mSockRef, buffer, length, 0);

	ASSERT(bytes > -1, GetError().c_str());
	return bytes;
}

int Socket::ReceiveFrom(void* data, sockaddr* address, socklen_t* addressSize, unsigned int length, unsigned int offset) const noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "Socket is not opened!");

	char* buffer = (char*)data + offset;

	int bytes = recvfrom(*mSockRef, buffer, length, 0, address, addressSize);
	ASSERT(bytes > -1, GetError().c_str());
	return bytes;
}

Socket::Socket(Socket&& other) noexcept
	: Socket()
{
	mSockRef = other.mSockRef;
	mAF = other.mAF;
	mBlockMode = other.mBlockMode;

	other.mSockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
	other.mAF = AF_UNSPEC;
	other.mBlockMode = true;
}

Socket& Socket::operator=(Socket&& rhs) noexcept
{
	if (this == &rhs) return *this;

	if ((mSockRef.use_count() == 1) && (*mSockRef != INVALID_SOCKET))
		Close();

	mSockRef = rhs.mSockRef;
	mAF = rhs.mAF;
	mBlockMode = rhs.mBlockMode;

	rhs.mSockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
	rhs.mAF = AF_UNSPEC;
	rhs.mBlockMode = true;
	return *this;
}

Socket::~Socket(void) noexcept
{
	if ((mSockRef.use_count() == 1) && (*mSockRef != INVALID_SOCKET))
		Close();
}

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

// ************************************************************************
// | Bellow From here the implementation differ depending on the Platform |
// ************************************************************************

void Socket::SetBlocking(bool flag) noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "Socket is not opened!");
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
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
	mBlockMode = flag;
}

void Socket::SetTimeout(unsigned long long milis) noexcept
{
	ASSERT(*mSockRef != INVALID_SOCKET, "Socket is not opened!");
	int result;
#ifdef PLATFORM_WINDOWS
	result = setsockopt(*mSockRef, SOL_SOCKET, SO_SNDTIMEO, (char*)&milis, sizeof(unsigned long long));
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
	result = setsockopt(*mSockRef, SOL_SOCKET, SO_RCVTIMEO, (char*)&milis, sizeof(unsigned long long));
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
#else
	timeval timeout;
	timeout.tv_usec = milis * 1000.0f;
	result = setsockopt(*mSockRef, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeval));
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
	result = setsockopt(*mSockRef, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeval));
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
#endif
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