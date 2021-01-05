#ifndef _SERVER
#define _SERVER

#include "Client.h"

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

namespace TCP {

	/**
	* @brief Object that provides server operations
	* @details A wrapper over the Socket class that helps
	*	on server operations like accepting new clients and
	*	creating new threads to handle their requests.
	*
	* @author Vasileios Mormoris
	* @version 1.0.0
	* @date 2020
	* @copyright MIT
	*/
	class Server {
	public:

		//Constructor(s) & Destructor
		Server(void) = default;
		~Server(void) = default;
		/**
		* @brief Copy contructor
		* @param other The server that will be copied
		*/
		Server(const Server&) noexcept;
		
		/**
		* @brief Move Constructor
		* @details Constructs a Server by moving a context from another one
		* @param other An r-value of a Server
		* @warning The other Server is becoming like a default contructed one
		*/
		Server(Server&& other) noexcept;

		/**
		* @brief Constructs a Server and open it's Socket
		* @param family Address family for the Server
		*/
		Server(int family) noexcept;

		/**
		* @brief Constructs a Server and binds it's Socket and putting in listening mode
		* @param family Adress Family that the Server will use
		* @param address The IP Address of the Server
		* @param port Port number of the Server
		* @warnings Assertion may occurried:
		*	- if the address is not the same family as the Server is opened with
		*/
		Server(int family, const char* address, unsigned short port, int qlen = SOMAXCONN) noexcept;

		/**
		* @brief Constructs a Server and binds it's Socket and putting in listening mode
		* @param family Adress Family that the Server will use
		* @param address A pair of IPv4 Address and Port of the Server
		* @warnings Assertion may occurried:
		*	- if family not specified AF_INET
		*/
		Server(const sockaddr_in& address, int qlen = SOMAXCONN) noexcept;

		/**
		* @brief Constructs a Server and binds it's Socket and putting in listening mode
		* @param family Adress Family that the Server will use
		* @param address A pair of IPv6 Address and Port of the Server
		* @warnings Assertion may occurried:
		*	- if family not specified AF_INET6
		*/
		Server(const sockaddr_in6& address, int qlen = SOMAXCONN) noexcept;

		/**
		* @brief Opens the Server's Socket
		* @param family Address family for the Server
		*/
		void Open(int family) noexcept;

		/**
		* @brief Binds the Server
		* @param address Address that the Server will bind to
		* @param port Port number that the Server will bind to
		* @warning Assertion may occuried:
		*	- if Server is already bound
		*	- if Server is not opened
		*	- if Server is opened with not the same Address Family as address
		*/
		void Bind(const char* address, unsigned short port) noexcept;

		/**
		* @brief Binds the Server
		* @param address A pair of IPv4 Address and Port
		* @warning Assertion may occuried:
		*	- if Server is already bound
		*	- if Server is not opened
		*	- if Server is opened with not IPv4 Address Family
		*/
		void Bind(const sockaddr_in& address) noexcept;

		/**
		* @brief Binds the Server
		* @param address A pair of IPv6 Address and Port
		* @warning Assertion may occuried:
		*	- if Server is already bound
		*	- if Server is not opened
		*	- if Server is opened with not IPv6 Address Family
		*/
		void Bind(const sockaddr_in6& address) noexcept;

		/**
		* @brief Closes the Server's socket 
		*/
		void Close(void) noexcept;

		/**
		* @brief Places the Server in a state in which it is listening for an incoming connection
		* @param length The maximum size of the queue of pending connections
		* @warning Assertion may occuried:
		*	- if the Server is not bound
		*	- if the Server is already on listening or connected state
		*/
		void Listen(int length = SOMAXCONN) noexcept;

		template<typename Func>
		void Start(Func&& func) noexcept
		{
			m_Component->Running = true;
			m_Component->Handle = std::thread([&](ServerComponent* component) {
				timeval timeout{ 1,0 };
				fd_set readset;
				SOCKET fd = m_Sock.GetNativeFD();

				while (true)
				{
					component->PoolMutex.lock();
					if (!component->Running)
					{
						component->PoolMutex.unlock();
						break;
					}
					component->PoolMutex.unlock();

					FD_ZERO(&readset);
					FD_SET(fd, &readset);

					int total = select(0, &readset, NULL, NULL, &timeout);
					ASSERT(total != SOCKET_ERROR, "Function select() failed!");
					if (!total) continue;//No Client is waiting to be accepted

					std::thread handle;
					
					if constexpr (std::is_same<FUNC_ARG<Func>, SOCKET>::value)
					{
						SOCKET sock = accept(m_Sock.GetNativeFD(), NULL, NULL);
						ASSERT(sock != INVALID_SOCKET, "Function accept() Failed!");
						handle = std::thread(func, sock);
					}
					else if constexpr (std::is_same<FUNC_ARG<Func>, Socket>::value)
						handle = std::thread(func, m_Sock.Accept());
					else if constexpr (std::is_same<FUNC_ARG<Func>, Client>::value)
					{
						Client client;
						client.m_Sock = m_Sock.Accept();
						handle = std::thread(func, client);
					}
					else { ASSERT(false, "Function signature is not valid!"); }
					handle.detach();
				}
			}, m_Component.get());
		}

		template<typename Func>
		void Start(Func&& func, int poolsize) noexcept
		{
			m_Component->PoolSize = 0;
			m_Component->Running = true;
			for (int i = 0; i < poolsize; i++)
			{
				std::thread handle = std::thread(&Server::ClientHandler<Func>, func, m_Component.get());
				handle.detach();
			}

			m_Component->Handle = std::thread([&](ServerComponent* component) {
				timeval timeout{ 1,0 };
				
				{
					std::unique_lock PoolLock(component->PoolMutex);
					component->PoolSize++;
					component->StartCV.notify_one();
				}
				
				fd_set readset;
				SOCKET fd = m_Sock.GetNativeFD();
				
				while (true)
				{
					component->PoolMutex.lock();
					if (!component->Running)
					{
						component->PoolMutex.unlock();
						break;
					}
					component->PoolMutex.unlock();
					
					FD_ZERO(&readset);
					FD_SET(fd, &readset);

					int total = select(0, &readset, NULL, NULL, &timeout);
					ASSERT(total != SOCKET_ERROR, "Function select() failed!");
					if (!total) continue;//No Client is waiting to be accepted

					Client client;
					client.m_Sock = m_Sock.Accept();

					{
						std::unique_lock PoolLock(component->PoolMutex);
						component->Clients.push(client);
						component->RunCV.notify_one();
					}
				}

			}, m_Component.get());
			
			//Wait for Client Handler threads to start
			//	as well as the Acceptor thread
			std::unique_lock PoolLock(m_Component->PoolMutex);
			while (m_Component->PoolSize < poolsize+1)
				m_Component->StartCV.wait(PoolLock);

		}

		/**
		* @brief Stops Server's currently working threads o
		*/
		void Stop(void) noexcept;

		/**
		* @brief Blocks until Server's acceptor thread is done
		*/
		void Join(void) noexcept;

		Socket& GetSocket(void) noexcept;
		const Socket& GetSocket(void) const noexcept;

		SOCKET GetNativeFD(void) const noexcept;

		/**
		* @brief Move assignement operation
		* @details Close the current Server if it is opened and move the context from another one to it
		* @param rhs An r-value of a Server
		* @returns A reference to a Server object
		* @warning The rhs Server is becoming like a default contructed one
		*/
		Server& operator=(Server&& rhs) noexcept;

		/**
		* @brief Copy assignement operation
		* @param rhs The server that will be copied
		* @returns A reference to a Server object
		*/
		Server& operator=(const Server& rhs) noexcept;

	private:

		struct ServerComponent {
			std::thread Handle;
			bool Running = false;
			std::queue<Client> Clients;
			std::mutex PoolMutex;
			std::condition_variable RunCV;
			std::condition_variable StartCV;
			int PoolSize = 0;
		};

	private:

		template<typename Func>
		static void ClientHandler(Func&& func, ServerComponent* component) noexcept
		{
			{//Inform main thread that a new thread started
				std::unique_lock PoolLock(component->PoolMutex);
				component->PoolSize++;
				component->StartCV.notify_one();
			}

			while (true)
			{
				Client client;
				
				{//Wait for new Client or a stop signal
					std::unique_lock PoolLock(component->PoolMutex);
					while (component->Running && component->Clients.empty())
						component->RunCV.wait(PoolLock);
					if (!component->Running)
						return;
					client = component->Clients.front();
					component->Clients.pop();
				}

				if constexpr (std::is_same<FUNC_ARG<Func>, Client>::value)
				{
					func(client);
				}
				else if constexpr (std::is_same<FUNC_ARG<Func>, Socket>::value)
				{
					func(client.GetSocket());
				}
				else if constexpr (std::is_same<FUNC_ARG<Func>, SOCKET>::value)
				{
					func(client.GetSocket().GetNativeFD());
				}
				else { ASSERT(false, "Function signature is not correct!"); }
			}
		}

	private:

		Socket m_Sock;
		std::shared_ptr<ServerComponent> m_Component = std::make_shared<ServerComponent>();

	};

}

#endif