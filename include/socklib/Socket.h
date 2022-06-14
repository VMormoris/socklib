#pragma once

#include <socklib/Platform.h>
#include <memory>
#include <string>

/**
* @brief Kilo Byte
*/
#define KiB 1024
/**
* @brief Mega Byte
*/
#define MiB 1024*KiB
/**
* @brief Giga Byte
*/
#define GiB 1024*MiB

namespace socklib {

	typedef unsigned char BYTE;

	/**
	 * @brief Basically a pair of IP and port
	 * @details This structure is meant to be used as syntax sugar
	 * 	for some of the Socket methods such as SendTo
	 */
	struct Endpoint{
		/**
		 * @brief Host's IP Address
		 */
		const char* Host = NULL;
		/**
		 * @brief Port number
		 */
		unsigned short Port = 0;
		//Constructor(s)
		Endpoint() = default;
		Endpoint(const std::string& host, unsigned short port)
			: Host(host.c_str()), Port(port) {}
		Endpoint(const char* host, unsigned short port)
			: Host(host), Port(port) {}

	};

	/**
	* @brief A platform agnostic Socket object
	* @details A python like socket object that also support low level functionality.
	*	Object are automatically closed when all references are invalidated, but it is
	*	recommended to Close() explicitly.
	*/
	class Socket {
	public:

		//Constructor(s) & Destructor
		Socket(void) noexcept = default;
		Socket(const Socket&) = default;
		~Socket(void) noexcept;

		/**
		* @brief Constructs a Socket object and calls Open
		* @param family Address family for the socket
		* @param type The socket's type
		* @param proto The protocol to be used by the socket
		* @sa Open method
		*/
		Socket(int family, int type, int proto = 0) noexcept
			: Socket() { Open(family, type, proto); }

		/**
		* @brief Move Constructor
		* @details Constructs a Socket by moving a context from another one, other
		*	Socket is being like a default contructed one
		* @param other An r-value of a Socket
		*/
		Socket(Socket&& other) noexcept;

		/**
		* @brief Opens the socket
		* @details Opens the socket using the given address family, socket type and protocol number.
		* @param family Address family for the socket
		* @param type The socket's type
		* @param proto The protocol to be used by the socket
		*/
		void Open(int family, int type, int proto = 0) noexcept;

		/**
		* @brief Binds the socket
		* @param address C-like string address that the socket will bind to
		* @param port Port number that the socket will bind to
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Bind(const char* address = NULL, unsigned short port = 0) const noexcept;

		/**
		* @brief Binds the socket
		* @param address String Address that the socket will bind to
		* @param port Port number that the socket will bind to
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Bind(const std::string& address, unsigned short port = 0) const noexcept;

		/**
		* @brief Binds the socket
		* @param address Pointer to the address that you want to bound (Basically a pair of address and port number)
		* @param size Size in bytes of the address structure
		*/
		void Bind(const sockaddr* address, socklen_t size) const noexcept;

		/**
		* @brief Disables sends or receives on a socket
		* @param how Specification for what should be disabled (reception, transmission, or both)
		*/
		void Shutdown(int how) noexcept;

		/**
		* @brief Closes an already open socket
		*/
		void Close(void) noexcept;

		/**
		* @brief Establish connection with another socket
		* @param address C-like string address of the remote host
		* @param port Port numberfor the process of the remote host
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Connect(const char* address, unsigned short port) const noexcept;

		/**
		* @brief Establish connection with another socket
		* @param address String address of the remote host
		* @param port Port numberfor the process of the remote host
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Connect(const std::string& address, unsigned short port) const noexcept;

		/**
		* @brief Establish connection with another socket
		* @param address Pointer to socket address of the remote host proccess (Basically a pair of address and port number)
		* @param size Size in bytes of the address structure
		*/
		void Connect(const sockaddr* address, socklen_t size) const noexcept;

		/**
		* @brief Places the socket in a state in which it is listening for an incoming connection
		* @param length The maximum size of the queue of pending connections
		*/
		void Listen(int length = SOMAXCONN) noexcept;

		/**
		* @brief Accepts a new connection
		* @return A Socket that can be used for sending and receiving data together with the client address and port number
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		std::pair<Socket, std::pair<std::string, unsigned short>> Accept(void) const noexcept;

		/**
		* @brief Accepts a new connection
		* @param[out] adress Pointer that hold the address of the new client (Basically a pair of address and port number)
		* @param[out] size Pointer that hold the size of client's address
		*/
		Socket Accept(sockaddr* address, socklen_t* size) const noexcept;

		/**
		* @brief Sends data to the connected socket
		* @param data Pointer to the data buffer that will be send
		* @param length Number of bytes that will be send
		* @param offset How many bytes away from the start should we start sending
		* @return The number of bytes that were actually send
		*/
		int Send(const void* data, size_t length, size_t offset = 0) const noexcept;
		
		/**
		* @brief Sends data to the specified address
		* @param data Pointer to the data buffer that will be send
		* @param address Pointer to socket address of the remote host proccess (Basically a pair of address and port number)
		* @param addressSize Size in bytes of the address structure
		* @param length Number of bytes that will be send
		* @param offset How many bytes away from the start should we start sending
		* @return The number of bytes that were actually send
		*/
		int SendTo(const void* data, const sockaddr* address, socklen_t addressSize, size_t length, size_t offset = 0) const noexcept;

		/**
		 * @brief Sends data to the specified address
		 * @param data Pointer to the data buffer that will be send
		 * @param endpoint Pair of remote host address and port
		 * @param length Number of bytes that will be send
		 * @param offset How many bytes away from the start should we start sending
		 * @return The number of bytes that were actually send
		 * @warning Only IPv4 and IPv6 are supported currently
		 * @sa Endpoint
		 */
		int SendTo(const void* data, const Endpoint& endpoint, size_t length, size_t offset = 0) const noexcept;
		
		/**
		* @brief Sends data from the connected socket
		* @param[out] data Pointer to the data that will be received
		* @param[in] length Number of bytes that will be received
		* @param[in] offset How many bytes away from the start should we start receiving
		* @return The number of bytes that were actually received
		*/
		int Receive(void* data, size_t length, size_t offset = 0) const noexcept;

		/**
		* @brief Receives data
		* @param[out] data Pointer to the data that will be received
		* @param[out] adress Pointer that hold the address of the new client (Basically a pair of address and port number)
		* @param[out] size Pointer that hold the size of client's address
		* @param[in] length Number of bytes that will be received
		* @param[in] offset How many bytes away from the start should we start receiving
		* @return The number of bytes that were actually received
		*/
		int ReceiveFrom(void* data, sockaddr* address, socklen_t* addressSize, size_t length, size_t offset = 0) const noexcept;

		/**
		* @brief Receives data
		* @param[out] data Pointer to the data that will be received
		* @param[in] length Number of bytes that will be received
		* @param[in] offset How many bytes away from the start should we start receiving
		* @return The number of bytes that were actually received togther with remote host address and port number
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		std::pair<int, std::pair<std::string, unsigned short>> ReceiveFrom(void* data, size_t length, size_t offset = 0) const noexcept;

		/**
		* @brief Setter for blocking mode
		* @param flag A bool false for non-blocking mode, true for blocking mode
		*/
		void SetBlocking(bool flag) noexcept;

		/**
		* @brief Setter for socket's time out
		* @param milis Miliseconds that the socket should wait before a timeout occuries
		*/
		void SetTimeout(uint64_t milis) noexcept;

		/**
		* @brief Getter for Native File Descriptor
		* @returns The file descriptor of the socket
		*/
		SOCKET Fileno(void) const noexcept;

		/**
		* @brief Move assignement operation
		* @details Close the current Socket if it is opened, and move the context from another one to it, other
		*	Socket is being like a default contructed one
		* @param other An r-value of a Socket
		*/
		Socket& operator=(Socket&& rhs) noexcept;

		Socket& operator=(const Socket&) = default;
	
	public:
		
		/**
		 * @brief Connects to a TCP service on the specified address
		 * @param family Address family for the socket
		 * @param endpoint Pair of IP and port number of ther remote host that will attempt to connect 
		 * @param timeout Optional parameter to set a timeout option on the socket
		 * @param local Optional parameter to specify the local address that will be used by client
		 * @return Socket after the requested connection has been established 
		 * @warning Only IPv4 and IPv6 are supported currently
		 */
		static Socket CreateConnection(int family, const Endpoint& endpoint, uint64_t timeout = 0, const Endpoint& local = {}) noexcept;

		/**
		 * @brief Convience function for creatign a TCP server
		 * @param family Address family for the socket
		 * @param endpoint Pair of IP and port number of ther remote host that will attempt to connect 
		 * @param queue Parameter that will be pass to Listen function
		 * @return Socket ready to accept new client connections
		 * @warning Only IPv4 and IPv6 are supported currently
		 */
		static Socket CreateServer(int family, const Endpoint& endpoint, int queue = SOMAXCONN) noexcept;

	private:

		std::shared_ptr<SOCKET> mSockRef;
		
		bool mBlockMode = true;

		int mAF = AF_UNSPEC;

	};

}