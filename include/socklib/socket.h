#ifndef SOCKET_H
#define SOCKET_H

#include <socklib/platform.h>
#include <memory>

typedef unsigned char BYTE;

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

/**
* @brief A platform agnostic Socket object
* @details A python like socket object that also support low level functionality
*/
class Socket {
public:

	//Constructor(s) & Destructor
	Socket(void) noexcept;
	Socket(const Socket&) = default;
	~Socket(void) noexcept;

	/**
	* @brief Constructs a Socket object and calls Open
	* @param family Address family for the socket
	* @param type The socket's type
	* @param proto The protocol to be used by the socket
	*/
	Socket(int family, int type, int proto = 0) noexcept
		: Socket() { Open(family, type, proto); }

	/**
	* @brief Move Constructor
	* @details Constructs a Socket by moving a context from another one
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
	* @param address Address that the socket will bind to
	* @param port Port number that the socket will bind to
	* @warning Only IPv4 and IPv6 are supported currently
	*/
	void Bind(const char* address = NULL, unsigned short port = 0) noexcept;

	/**
	* @brief Binds the socket
	* @param address A pair of IPv4 Address and Port
	*/
	void Bind(const sockaddr_in& address) noexcept;

	/**
	* @brief Binds the socket
	* @param address A pair of IPv6 Address and Port
	*/
	void Bind(const sockaddr_in6& address) noexcept;


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
	* @param address The IP Address of the remote host
	* @param port Port numberfor the process of the remote host
	* @warning Only IPv4 and IPv6 are supported currently
	*/
	void Connect(const char* address, unsigned short port) noexcept;

	/**
	* @brief Establish connection with another socket
	* @param address A pair of IPv4 Address and Port of the remote host process
	*/
	void Connect(const sockaddr_in& address) noexcept;

	/**
	* @brief Establish connection with another socket
	* @param address A pair of IPv6 Address and Port of the remote host process
	*/
	void Connect(const sockaddr_in6& address) noexcept;

	/**
	* @brief Places the socket in a state in which it is listening for an incoming connection
	* @param length The maximum size of the queue of pending connections
	*/
	void Listen(int length = SOMAXCONN) noexcept;

	/**
	* @brief Accepts a new connection
	* @returns A Socket that can be used for sending and receiving data
	*/
	Socket Accept(void) const noexcept;

	/**
	* @brief Accepts a new connection
	* @param address IPv4 pair of address & port to be filled with who was accepted 
	* @returns A Socket that can be used for sending and receiving data
	*/
	Socket Accept(sockaddr_in& address) const noexcept;

	/**
	* @brief Accepts a new connection
	* @param address IPv6 pair of address & port to be filled with who was accepted
	* @returns A Socket that can be used for sending and receiving data
	*/
	Socket Accept(sockaddr_in6& address) const noexcept;

	/**
	* @brief Sends data to the connected socket
	* @param data Pointer to the data that will be send
	* @param length Number of bytes that will be send
	* @param offset How many bytes away from the start should we start sending
	*/
	int Send(const void* data, unsigned int length, unsigned int offset = 0) const noexcept;

	/**
	* @brief Sends data to the specified address
	* @param data Pointer to the data that will be send
	* @param address IPv4 pair of address & port
	* @param length Number of bytes that will be send
	* @param offset How many bytes away from the start should we start sending
	*/
	int SendTo(const void* data, const sockaddr_in& address, unsigned int length, unsigned int offset = 0) const noexcept;

	/**
	* @brief Sends data to the specified address
	* @param data Pointer to the data that will be send
	* @param address IPv6 pair of address & port
	* @param length Number of bytes that will be send
	* @param offset How many bytes away from the start should we start sending
	*/
	int SendTo(const void* data, const sockaddr_in6& address, unsigned int length, unsigned int offset = 0) const noexcept;

	/**
	* @brief Sends data from the connected socket
	* @param data Pointer to the data that will be received
	* @param length Number of bytes that will be received
	* @param offset How many bytes away from the start should we start receiving
	*/
	int Receive(void* data, unsigned int length, unsigned int offset = 0) const noexcept;


	/**
	* @brief Receives data
	* @param data Pointer to the data that will be received
	* @param address IPv4 pair of address & port
	* @param length Number of bytes that will be received
	* @param offset How many bytes away from the start should we start receiving
	*/
	int ReceiveFrom(void* data, sockaddr_in& address, unsigned int length, unsigned int offset = 0) const noexcept;
	
	/**
	* @brief Receives data
	* @param data Pointer to the data that will be received
	* @param address IPv6 pair of address & port
	* @param length Number of bytes that will be received
	* @param offset How many bytes away from the start should we start receiving
	*/
	int ReceiveFrom(void* data, sockaddr_in6& address, unsigned int length, unsigned int offset = 0) const noexcept;

	/**
	* @brief Setter for blocking mode
	* @param flag A bool false for non-blocking mode, true for blocking mode
	*/
	void SetBlocking(bool flag) noexcept;

	/**
	* @brief Setter for socket's time out
	* @param milis Miliseconds that the socket should wait before a timeout occuries
	*/
	void SetTimeout(unsigned long long milis) noexcept;

	/**
	* @brief Getter for Native File Descriptor
	* @returns The file descriptor of the socket
	*/
	SOCKET GetNativeFD(void) const noexcept { return *m_SockRef; }

	/**
	* @brief Move assignement operation
	* @details Close the current Socket if it is opened, and move the context from another one to it
	* @param other An r-value of a Socket
	*/
	Socket& operator=(Socket&& rhs) noexcept;

	Socket& operator=(const Socket&) = default;

private:

	std::shared_ptr<SOCKET> m_SockRef;
	
	bool m_BlockMode = true;

	int m_AF = AF_UNSPEC;

};

#endif