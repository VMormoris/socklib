#pragma once

#include <socklib/Platform.h>
#include <memory>
#include <string>

namespace socklib {

	/**
	* @brief Kilo Byte
	*/
	constexpr size_t KiB = 1024;
	/**
	* @brief Mega Byte
	*/
	constexpr size_t MiB = 1024 * KiB;
	/**
	* @brief Giga Byte
	*/
	constexpr size_t GiB = 1024 * MiB;

	typedef unsigned char BYTE;

	enum class AddressFamily : int {
		UNSPECIFIED = AF_UNSPEC, // Not supported
		UNIX = AF_UNIX, // Not supported
		IPv4 = AF_INET,
		IPv6 = AF_INET6,
		BLUETOOTH = AF_BLUETOOTH, // Not supported yet
	};

	enum class SocketType : int {
		UNSPECIFIED = 0,
		STREAM = SOCK_STREAM,
		DGRAM = SOCK_DGRAM,
		RAW = SOCK_RAW, // Not supported
		RDM = SOCK_RDM, // Not supported
		SEQ_PACKET = SOCK_SEQPACKET, // Not supported
		DCCP = SOCK_DCCP, // Not supported
		PACKET = SOCK_PACKET, // Not supported
		CLOEXEC = SOCK_CLOEXEC, // Not supported
		NONBLOCK = SOCK_NONBLOCK // Not supported
	};

	/**
	 * @brief Basically a pair of IP and port
	 * @details This structure is meant to be used as syntax sugar
	 * 	for some of the Socket methods such as SendTo
	 */
	struct Endpoint {
		/**
		 * @brief Host's IP Address
		 */
		std::string Host = "0.0.0.0";
		/**
		 * @brief Port number
		 */
		unsigned short Port = 0;
		//Constructor(s)
		Endpoint() = default;
		Endpoint(const std::string& host, const unsigned short port)
			: Host(host), Port(port) {}
		Endpoint(std::string&& host, const unsigned short port)
			: Host(std::move(host)), Port(port) {}
		Endpoint(const char* host, const unsigned short port)
			: Host(host), Port(port) {}

	};

	/**
	* @brief A platform-agnostic Socket object
	* @details A python like socket object that also support low level functionality.
	*	Object are automatically closed when all references are invalidated, but it is
	*	recommended to Close() explicitly. The implementation basically is wrapper over
	*	C-API platform specific calls, that uses share_ptr on the file descriptor of a socket
	*/
	class Socket {
	public:

		//Constructor(s) & Destructor
		Socket() noexcept = default;
		Socket(const Socket&) = default;
		~Socket() noexcept;

		/**
		* @brief Constructs a Socket object and calls Open
		* @param family Address family for the socket
		* @param type The socket's type
		* @param proto The protocol to be used by the socket
		* @sa Open method
		*/
		Socket(const AddressFamily family, const SocketType type, const int proto = 0) noexcept
			: Socket() { Open(family, type, proto); }

		/**
		* @brief Move Constructor
		* @details Constructs a Socket by moving a context from another one, other
		*	Socket is being like a default constructed one
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
		void Open(AddressFamily family, SocketType type, int proto = 0) noexcept;

		/**
		* @brief Binds the socket
		* @param endpoint Endpoint structure, that defines the address and port to bind
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Bind(const Endpoint& endpoint) const noexcept { Bind(endpoint.Host.c_str(), endpoint.Port); }

		/**
		* @brief Binds the socket
		* @param address C-like string address that the socket will bind to
		* @param port Port number that the socket will bind to
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Bind(const char* address = nullptr, unsigned short port = 0) const noexcept;

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
		void Shutdown(int how) const noexcept;

		/**
		* @brief Closes an already open socket
		*/
		void Close() noexcept;

		/**
		* @brief Establish connection with another socket
		* @param endpoint Endpoint structure, that defines the address and port to connect
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Connect(const Endpoint& endpoint) const noexcept;

		/**
		* @brief Establish connection with another socket
		* @param address C-like string address of the remote host
		* @param port Port number the process of the remote host
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Connect(const char* address, unsigned short port) const noexcept;

		/**
		* @brief Establish connection with another socket
		* @param address String address of the remote host
		* @param port Port number for the process of the remote host
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		void Connect(const std::string& address, unsigned short port) const noexcept;

		/**
		* @brief Establish connection with another socket
		* @param address Pointer to socket address of the remote host process (Basically a pair of address and port number)
		* @param size Size in bytes of the address structure
		*/
		void Connect(const sockaddr* address, socklen_t size) const noexcept;

		/**
		* @brief Places the socket in a state in which it is listening for an incoming connection
		* @param length The maximum size of the queue of pending connections
		*/
		void Listen(int length = SOMAXCONN) const noexcept;

		/**
		* @brief Accepts a new connection
		* @return A Socket that can be used for sending and receiving data together with the client address and port number
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		[[nodiscard]] std::pair<Socket, Endpoint> Accept() const noexcept;

		/**
		* @brief Accepts a new connection
		* @param[out] address Pointer that hold the address of the new client (Basically a pair of address and port number)
		* @param[out] size Pointer that hold the size of client's address
		*/
		Socket Accept(sockaddr* address, socklen_t* size) const noexcept;

		/**
		* @brief Sends data to the connected socket
		* @param data Pointer to the data buffer that will be sent
		* @param length Number of bytes that will be sent
		* @param offset How many bytes away from the start should we start sending
		* @return The number of bytes that were actually send
		*/
		IOSize Send(const void* data, size_t length, size_t offset = 0) const noexcept;
		
		/**
		* @brief Sends data to the specified address
		* @param data Pointer to the data buffer that will be sent
		* @param address Pointer to socket address of the remote host process (Basically a pair of address and port number)
		* @param addressSize Size in bytes of the address structure
		* @param length Number of bytes that will be sent
		* @param offset How many bytes away from the start should we start sending
		* @return The number of bytes that were actually send
		*/
		IOSize SendTo(const void* data, const sockaddr* address, socklen_t addressSize, size_t length, size_t offset = 0) const noexcept;

		/**
		 * @brief Sends data to the specified address
		 * @param data Pointer to the data buffer that will be sent
		 * @param endpoint Pair of remote host address and port
		 * @param length Number of bytes that will be sent
		 * @param offset How many bytes away from the start should we start sending
		 * @return The number of bytes that were actually send
		 * @warning Only IPv4 and IPv6 are supported currently
		 * @sa Endpoint
		 */
		IOSize SendTo(const void* data, const Endpoint& endpoint, size_t length, size_t offset = 0) const noexcept;
		
		/**
		* @brief Sends data from the connected socket
		* @param[out] data Pointer to the data that will be received
		* @param[in] length Number of bytes that will be received
		* @param[in] offset How many bytes away from the start should we start receiving
		* @return The number of bytes that were actually received
		*/
		IOSize Receive(void* data, size_t length, size_t offset = 0) const noexcept;

		/**
		* @brief Receives data
		* @param[out] data Pointer to the data that will be received
		* @param[out] address Pointer that hold the address of the new client (Basically a pair of address and port number)
		* @param[out] addressSize Pointer that hold the size of client's address
		* @param[in] length Number of bytes that will be received
		* @param[in] offset How many bytes away from the start should we start receiving
		* @return The number of bytes that were actually received
		*/
		IOSize ReceiveFrom(void* data, sockaddr* address, socklen_t* addressSize, size_t length, size_t offset = 0) const noexcept;

		/**
		* @brief Receives data
		* @param[out] data Pointer to the data that will be received
		* @param[in] length Number of bytes that will be received
		* @param[in] offset How many bytes away from the start should we start receiving
		* @return The number of bytes that were actually received together with remote host address and port number
		* @warning Only IPv4 and IPv6 are supported currently
		*/
		std::pair<IOSize, Endpoint> ReceiveFrom(void* data, size_t length, size_t offset = 0) const noexcept;

		/**
		* @brief Setter for blocking mode
		* @param flag A bool false for non-blocking mode, true for blocking mode
		*/
		void SetBlocking(bool flag) noexcept;

		/**
		* @brief Setter for socket's time out
		* @param millis Milliseconds that the socket should wait before a timeout occurs
		*/
		void SetTimeout(uint64_t millis) const noexcept;

		/**
		* @brief Getter for Native File Descriptor
		* @returns The file descriptor of the socket
		*/
		[[nodiscard]] SOCKET FileNo() const noexcept;

		/**
		* @brief Move assignment operation
		* @details Close the current Socket if it is opened, and move the context from another one to it, other
		*	Socket is being like a default contracted one
		* @param rhs An r-value of a Socket
		*/
		Socket& operator=(Socket&& rhs) noexcept;

		Socket& operator=(const Socket&) = default;

		[[nodiscard]] bool IsBlocking() const noexcept { return mBlockMode; }
	
	public:
		
		/**
		 * @brief Connects to a TCP service on the specified address
		 * @param family Address family for the socket
		 * @param endpoint Pair of IP and port number of their remote host that will attempt to connect
		 * @param timeout Optional parameter to set a timeout option on the socket
		 * @param local Optional parameter to specify the local address that will be used by client
		 * @return Socket after the requested connection has been established 
		 * @warning Only IPv4 and IPv6 are supported currently
		 */
		static Socket CreateConnection(AddressFamily family, const Endpoint& endpoint, uint64_t timeout = 0, const Endpoint& local = {}) noexcept;

		/**
		 * @brief Convenient function for creation a TCP server
		 * @param family Address family for the socket
		 * @param endpoint Pair of IP and port number of their remote host that will attempt to connect
		 * @param queue Parameter that will be pass to Listen function
		 * @return Socket ready to accept new client connections
		 * @warning Only IPv4 and IPv6 are supported currently
		 */
		static Socket CreateServer(AddressFamily family, const Endpoint& endpoint, int queue = SOMAXCONN) noexcept;

	private:

		std::shared_ptr<SOCKET> mSockRef;
		
		bool mBlockMode = true;

		AddressFamily mAF = AddressFamily::UNSPECIFIED;

	};

}