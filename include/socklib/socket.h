#pragma once

#include <socklib/platform.h>
#include <memory>
#include <string>

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
	* @returns A Socket that can be used for sending and receiving data
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
	* @param data Pointer to the data that will be send
	* @param length Number of bytes that will be send
	* @param offset How many bytes away from the start should we start sending
	*/
	int Send(const void* data, unsigned int length, unsigned int offset = 0) const noexcept;
	
	/**
	* @brief Sends data to the specified address
	* @param data Pointer to the data that will be send
	* @param address Pointer to socket address of the remote host proccess (Basically a pair of address and port number)
	* @param addressSize Size in bytes of the address structure
	* @param length Number of bytes that will be send
	* @param offset How many bytes away from the start should we start sending
	*/
	int SendTo(const void* data, const sockaddr* address, socklen_t addressSize, unsigned int length, unsigned int offset = 0) const noexcept;

	/**
	* @brief Sends data from the connected socket
	* @param[out] data Pointer to the data that will be received
	* @param[in] length Number of bytes that will be received
	* @param[in] offset How many bytes away from the start should we start receiving
	*/
	int Receive(void* data, unsigned int length, unsigned int offset = 0) const noexcept;

	/**
	* @brief Receives data
	* @param[out] data Pointer to the data that will be received
	* @param[out] adress Pointer that hold the address of the new client (Basically a pair of address and port number)
	* @param[out] size Pointer that hold the size of client's address
	* @param[in] length Number of bytes that will be received
	* @param[in] offset How many bytes away from the start should we start receiving
	*/
	int ReceiveFrom(void* data, sockaddr* address, socklen_t* addressSize, unsigned int length, unsigned int offset = 0) const noexcept;

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
	SOCKET GetNativeFD(void) const noexcept { return *mSockRef; }

	/**
	* @brief Move assignement operation
	* @details Close the current Socket if it is opened, and move the context from another one to it
	* @param other An r-value of a Socket
	*/
	Socket& operator=(Socket&& rhs) noexcept;

	Socket& operator=(const Socket&) = default;

private:

	std::shared_ptr<SOCKET> mSockRef;
	
	bool mBlockMode = true;

	int mAF = AF_UNSPEC;

};