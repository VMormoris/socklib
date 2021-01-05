#include "Server.h"


namespace TCP {

	void Server::Stop(void) noexcept
	{
		std::unique_lock PoolLock(m_Component->PoolMutex);
		m_Component->Running = false;
		m_Component->RunCV.notify_all();
	}

	Server::Server(int family, const char* address, unsigned short port, int qlen) noexcept
	{
		Open(family);
		Bind(address, port);
		Listen(qlen);
	}

	Server::Server(const sockaddr_in& address, int qlen) noexcept
	{
		Open(AF_INET);
		Bind(address);
		Listen(qlen);
	}

	Server::Server(const sockaddr_in6& address, int qlen) noexcept
	{
		Open(AF_INET6);
		Bind(address);
		Listen(qlen);
	}

	void Server::Join(void) noexcept
	{
		m_Component->Handle.join();
	}

	Server::Server(int family) noexcept { Open(family); }

	void Server::Open(int family) noexcept { m_Sock.Open(family, SOCK_STREAM); }

	void Server::Bind(const char* address, unsigned short port) noexcept { m_Sock.Bind(address, port); }
	void Server::Bind(const sockaddr_in& address) noexcept { m_Sock.Bind(address); }
	void Server::Bind(const sockaddr_in6& address) noexcept { m_Sock.Bind(address); }

	void Server::Listen(int length) noexcept { m_Sock.Listen(length); }

	void Server::Close(void) noexcept { m_Sock.Close(); }

	Server::Server(const Server& other) noexcept
		: m_Sock(other.m_Sock), m_Component(other.m_Component) {}

	Server::Server(Server&& other) noexcept
		: m_Sock(std::move(other.m_Sock)), m_Component(other.m_Component)
	{
		other.m_Component = std::make_shared<ServerComponent>();
	}

	Server& Server::operator=(const Server& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_Sock = rhs.m_Sock;
			m_Component = rhs.m_Component;
		}
		return *this;
	}

	Server& Server::operator=(Server&& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_Sock = std::move(rhs.m_Sock);
			m_Component = rhs.m_Component;
			rhs.m_Component = std::make_shared<ServerComponent>();
		}
		return *this;
	}
	
	Socket& Server::GetSocket(void) noexcept { return m_Sock; }
	const Socket& Server::GetSocket(void) const noexcept { return m_Sock; }

	SOCKET Server::GetNativeFD(void) const noexcept { return m_Sock.GetNativeFD(); }


}

