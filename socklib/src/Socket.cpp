#include <socklib/socket.h>
#include <string>

//Declaration of helper functions
void CreateAddress(const char* address, unsigned short port, sockaddr_in& sockAddress) noexcept;
void CreateAddress(const char* address, unsigned short port, sockaddr_in6& sockAddress) noexcept;
std::string GetError(void) noexcept;
//End Declaration of helper functions


Socket::Socket(void) noexcept
{
	m_SockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
}

void Socket::Open(int family, int type, int proto) noexcept
{
	SOCKET& sock = *m_SockRef;
	ASSERT(sock == INVALID_SOCKET, "Socket is already opened!");
	sock = socket(family, type, proto);
	ASSERT(sock != INVALID_SOCKET, GetError().c_str());
	m_AF = family;
}

void Socket::Bind(const char* address, unsigned short port) noexcept
{
	switch (m_AF)
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

void Socket::Bind(const sockaddr* address, socklen_t size) noexcept
{
	ASSERT(m_AF == address->sa_family, "Socket hasn't opened with same address Family!");
	int result = bind(*m_SockRef, address, size);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
}

void Socket::Shutdown(int how) noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "The Socket is not opened!")
	int result = shutdown(*m_SockRef, how);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
}

void Socket::Close(void) noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "The Socket is already closed!");
	int result = closesocket(*m_SockRef);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
	*m_SockRef = INVALID_SOCKET;
	m_AF = AF_UNSPEC;
}

void Socket::Connect(const char* address, unsigned short port) noexcept
{
	switch (m_AF)
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

void Socket::Connect(const sockaddr* address, socklen_t size) noexcept
{
	ASSERT(m_AF == address->sa_family, "Socket hasn't opened with same address Family!");
	int result = connect(*m_SockRef, address, size);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
}

void Socket::Listen(int length) noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "The Socket is not opened!");
	int result = listen(*m_SockRef, length);
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
}

Socket Socket::Accept(void) const noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "The Socket is not opened!");
	Socket clientSock{};
	
	*clientSock.m_SockRef = accept(*m_SockRef, NULL, NULL);
	ASSERT(*clientSock.m_SockRef != INVALID_SOCKET, GetError().c_str());

	clientSock.m_AF = m_AF;
	return clientSock;
}

Socket Socket::Accept(sockaddr* address, socklen_t* size) const noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "The Socket is not opened!");

	Socket client{};
	*client.m_SockRef = accept(*m_SockRef, address, size);
	ASSERT(*client.m_SockRef != INVALID_SOCKET, GetError().c_str());
	client.m_AF = m_AF;
	return client;
}


int Socket::Send(const void* data, unsigned int length, unsigned int offset) const noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "Socket is not opened!");
	char* buffer = (char*)data + offset;

	int bytes = send(*m_SockRef, buffer, length, 0);

	ASSERT(bytes > -1, GetError().c_str());
	return bytes;
}

int Socket::SendTo(const void* data, const sockaddr* address, socklen_t addressSize, unsigned int length, unsigned offset) const noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "Socket is not opened!");
	ASSERT(m_AF == address->sa_family, "Socket hasn't opened with same address Family!");
	char* buffer = (char*)data + offset;

	int bytes = sendto(*m_SockRef, buffer, length, 0, address, addressSize);

	ASSERT(bytes > -1, GetError().c_str());
	return bytes;
}

int Socket::Receive(void* data, unsigned int length, unsigned int offset) const noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "Socket is not opened!");
	char* buffer = (char*)data + offset;

	int bytes = recv(*m_SockRef, buffer, length, 0);

	ASSERT(bytes > -1, GetError().c_str());
	return bytes;
}

int Socket::ReceiveFrom(void* data, sockaddr* address, socklen_t* addressSize, unsigned int length, unsigned int offset) const noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "Socket is not opened!");

	char* buffer = (char*)data + offset;

	int bytes = recvfrom(*m_SockRef, buffer, length, 0, address, addressSize);
	ASSERT(bytes > -1, GetError().c_str());
	return bytes;
}

Socket::Socket(Socket&& other) noexcept
	: Socket()
{
	m_SockRef = other.m_SockRef;
	m_AF = other.m_AF;
	m_BlockMode = other.m_BlockMode;

	other.m_SockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
	other.m_AF = AF_UNSPEC;
	other.m_BlockMode = true;
}

Socket& Socket::operator=(Socket&& rhs) noexcept
{
	if (this == &rhs) return *this;

	if ((m_SockRef.use_count() == 1) && (*m_SockRef != INVALID_SOCKET))
		Close();

	m_SockRef = rhs.m_SockRef;
	m_AF = rhs.m_AF;
	m_BlockMode = rhs.m_BlockMode;

	rhs.m_SockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
	rhs.m_AF = AF_UNSPEC;
	rhs.m_BlockMode = true;
	return *this;
}

Socket::~Socket(void) noexcept
{
	if ((m_SockRef.use_count() == 1) && (*m_SockRef != INVALID_SOCKET))
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
	ASSERT(*m_SockRef != INVALID_SOCKET, "Socket is not opened!");
	if (flag == m_BlockMode) return;
	int result = SOCKET_ERROR;
#ifdef PLATFORM_WINDOWS
	unsigned long iMode = (unsigned long)!flag;
	result = ioctlsocket(*m_SockRef, FIONBIO, &iMode);
#else
	int flags = fcntl(*m_SockRef, F_GETFL);
	flags = !flag ? (flags | O_NONBLOCK) : (flags & (~O_NONBLOCK));
	result = ioctl(*m_SockRef, F_SETFL, flags);
#endif
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
	m_BlockMode = flag;
}

void Socket::SetTimeout(unsigned long long milis) noexcept
{
	ASSERT(*m_SockRef != INVALID_SOCKET, "Socket is not opened!");
	int result;
#ifdef PLATFORM_WINDOWS
	result = setsockopt(*m_SockRef, SOL_SOCKET, SO_SNDTIMEO, (char*)&milis, sizeof(unsigned long long));
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
	result = setsockopt(*m_SockRef, SOL_SOCKET, SO_RCVTIMEO, (char*)&milis, sizeof(unsigned long long));
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
#else
	timeval timeout;
	timeout.tv_usec = milis * 1000.0f;
	result = setsockopt(*m_SockRef, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeval));
	ASSERT(result != SOCKET_ERROR, GetError().c_str());
	result = setsockopt(*m_SockRef, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeval));
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