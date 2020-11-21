#include "Client.h"

BasicClient::~BasicClient(void) noexcept
{
	ASSERT(m_State != ThreadState::Running, "Since thread started without a detached flag you should called Join before the destructor!");
}

BasicClient::BasicClient(const BasicClient& other) noexcept
	: m_Sock(other.m_Sock), m_State(other.m_State)
{
	ASSERT(m_State != ThreadState::Running || other.m_State == ThreadState::Running, "Copying to or from a Client with a running thread may lead to not desirable behavior!");
}

BasicClient::BasicClient(BasicClient&& other) noexcept
	: m_Sock(std::move(other.m_Sock)), m_Handle(std::move(other.m_Handle)), m_State(other.m_State)
{
	other.m_State = ThreadState::None;
}

BasicClient::BasicClient(int family, int socktype, int proto) noexcept
	: m_Sock(family, socktype, proto) {}


void BasicClient::Join(void) noexcept
{
	ASSERT(m_State == ThreadState::Running, "There is no Client thread running or the thread is detached!");
	m_Handle.join();
	m_State = ThreadState::None;
}

BasicClient& BasicClient::operator=(BasicClient&& rhs) noexcept
{
	if (this == &rhs) return *this;

	m_Sock = std::move(rhs.m_Sock);
	m_Handle = std::move(rhs.m_Handle);
	m_State = rhs.m_State;

	rhs.m_State = ThreadState::None;
	return *this;
}

BasicClient& BasicClient::operator=(const BasicClient& rhs) noexcept
{
	if (this == &rhs) return *this;
	ASSERT(rhs.m_State != ThreadState::Running || m_State != ThreadState::Running , "Copying from or to a Client with a running thread may lead undefined behavior!");
	m_Sock = rhs.m_Sock;
	m_State = rhs.m_State;

	return *this;
}


void BasicClient::Open(int family, int socktype, int proto) noexcept { m_Sock.Open(family, socktype, proto); }

void BasicClient::Bind(const char* address, unsigned short port) noexcept { m_Sock.Bind(address, port); }
void BasicClient::Bind(const sockaddr_in& address) noexcept { m_Sock.Bind(address); }
void BasicClient::Bind(const sockaddr_in6& address) noexcept { m_Sock.Bind(address); }

void BasicClient::Close(void) noexcept { m_Sock.Close(); }

void BasicClient::Shutdown(int how) noexcept { m_Sock.Shutdown(how); }

Socket& BasicClient::GetSocket(void) noexcept { return m_Sock; }
const Socket& BasicClient::GetSocket(void) const noexcept { return m_Sock; }

namespace TCP {

	Client::Client(int family) noexcept
		: BasicClient(family, SOCK_STREAM, 0) {}

	Client::Client(int family, const char* address, unsigned short port) noexcept
		: Client(family)
	{
		Connect(address, port);
	}

	Client::Client(const sockaddr_in& address) noexcept
		: Client(AF_INET)
	{
		Connect(address);
	}

	Client::Client(const sockaddr_in6& address) noexcept
		: Client(AF_INET6)
	{
		Connect(address);
	}

	void Client::Connect(const char* address, unsigned short port) noexcept { m_Sock.Connect(address, port); }
	void Client::Connect(const sockaddr_in& address) noexcept { m_Sock.Connect(address); }
	void Client::Connect(const sockaddr_in6& address) noexcept { m_Sock.Connect(address); }

	int Client::Send(const void* data, unsigned int length, unsigned int offset) const noexcept { return m_Sock.Send(data, length, offset); }
	int Client::Receive(void* data, unsigned int length, unsigned int offset) const noexcept { return m_Sock.Receive(data, length, offset); }

	void Client::Disconnect(void) noexcept
	{
		Shutdown(SHUT_WR);
		Close();
	}
}

namespace UDP {

	Client::Client(int family) noexcept
		: BasicClient(family, SOCK_DGRAM, 0) {}

	Client::Client(int family, const char* address, unsigned short port) noexcept
		: Client(family)
	{
		Bind(address, port);
	}

	Client::Client(const sockaddr_in& address) noexcept
		: Client(AF_INET)
	{
		Bind(address);
	}

	Client::Client(const sockaddr_in6& address) noexcept
		: Client(AF_INET6)
	{
		Bind(address);
	}

	int Client::SendTo(const void* data, const sockaddr_in& address, unsigned int length, unsigned int offset) const noexcept { return m_Sock.SendTo(data, address, length, offset); }
	int Client::SendTo(const void* data, const sockaddr_in6& address, unsigned int length, unsigned int offset) const noexcept { return m_Sock.SendTo(data, address, length, offset); }
	int Client::ReceiveFrom(void* data, sockaddr_in& address, unsigned int length, unsigned int offset) const noexcept { return m_Sock.ReceiveFrom(data, address, length, offset); }
	int Client::ReceiveFrom(void* data, sockaddr_in6& address, unsigned int length, unsigned int offset) const noexcept { return m_Sock.ReceiveFrom(data, address, length, offset); }

}
