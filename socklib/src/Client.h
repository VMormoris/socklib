#ifndef _Client
#define _Client

#include <type_traits>

template<typename Ret, typename Arg, typename ...Rest>
Arg helper(Ret(*) (Arg, Rest...));

template<typename Ret, typename Functor, typename Arg, typename ...Rest>
Arg helper(Ret(Functor::*) (Arg, Rest...));

template<typename Ret, typename Functor, typename Arg, typename ...Rest>
Arg helper(Ret(Functor::*) (Arg, Rest...) const);

template <typename Functor>
decltype(helper(&Functor::operator())) helper(Functor);

template <typename T>
using FUNC_ARG = decltype(helper(std::declval<T>()));

#include "Socket.h"
#include <thread>

/**
* @brief Object that provides with basic Client's operation
* @details Basic Client is wrapper over the Socket class that provides
*	basic Client's operations. This class purpose is to be used as parent
*	class from Client classes that are defined by in the apropiated
*	namespace according to the protocol.
*
* @warning Prefered to use a Client objects instead of this one
* @author Vasileios Mormoris
* @version 1.0.0
* @date 2020
* @copyright MIT
*/
class BasicClient {
public:

	//Constructor(s) & Destructor
	BasicClient(void) = default;
	virtual ~BasicClient(void) noexcept;

	/**
	* @brief Copy constructor
	* @details Constructs a Client by copying the context from another one to it
	* @param other Client that will be copied
	* @warning Generally calling copying operations on Clients should
	*	avoided look at the wiki for more info.
	*/
	BasicClient(const BasicClient& other) noexcept;

	/**
	* @brief Move Constructor
	* @details Constructs a Client by moving the context from another one to it
	* @param other An r-value of a Client
	* @warning The other Client is becoming like a default contructed one
	*/
	BasicClient(BasicClient&& other) noexcept;

	/**
	* @brief Constructs a Client and opens it's underlying Socket
	* @param family Address family for the Client
	* @param type The socket's type
	* @param proto The protocol to be used by the Client
	*/
	BasicClient(int family, int socktype, int proto = 0) noexcept;

	/**
	* @brief Opens the Client's underlying Socket
	* @details Opens the Client's socket using the given address family, socket type and protocol number.
	* @param family Address family for the Client
	* @param type The socket's type
	* @param proto The protocol to be used by the Client
	* @warning Only IPv4 & IPv6 are currently supported as Address families
	*			Assertion may occuried if Client's socket have been already closed or haven't be opened
	*/
	void Open(int family, int type, int proto = 0) noexcept;



	/**
	* @brief Binds the Client's underlying Socket
	* @param address Address that the Client will bind to
	* @param port Port number that the Client will bind to
	* @warning Assertion may occuried:
	*	- if Client's socket have been already closed or haven't be opened
	*	- if Client's socket is opened with not the same Address Family as address
	*/
	void Bind(const char* address = NULL, unsigned short port = 0) noexcept;

	/**
	* @brief Binds the Client's underlying Socket
	* @param address A pair of IPv4 Address and Port
	* @warning Assertion may occuried if Client's socket is opened with an Address Family other than IPv4
	*/
	void Bind(const sockaddr_in& address) noexcept;

	/**
	* @brief Binds the Client's underlying Socket
	* @param address A pair of IPv6 Address and Port
	* @warning Assertion may occuried if Client's socket is opened with an Address Family other than IPv6
	*/
	void Bind(const sockaddr_in6& address) noexcept;

	/**
	* @brief Disables sends or receives on a Client
	* @param how Specification for what should be disabled (reception or transmission)
	* @warning Assertion may occuried if Client's socket have been already closed or haven't be opened
	*/
	void Shutdown(int how) noexcept;

	/**
	* @brief Closes an already open Client
	* @warning Assertion may occuried if Client's socket have been already closed or haven't be opened
	*/
	void Close(void) noexcept;

	/**
	* @brief Getter for underlying Socket
	*/
	Socket& GetSocket(void) noexcept;

	/**
	* @brief Getter for underlying Socket
	*/
	const Socket& GetSocket(void) const noexcept;

	SOCKET GetNativeFD(void) const noexcept;


	/**
	* @brief Starts a new thread that will handle the Client's logic
	* 
	*	The function is signature should void(Socket) or void(SOCKET). Lamdas and functors with apropriate
	*	signatures are also acceptable. Starting a Client's thread as a detached one will invalidate the Client
	*	means that the Client will be like default contstructed one.
	* @tparam Func Type of the function/lamda that will be used on the new thread
	* @param func Function/Lamda that will be used by the newly created thread
	* @param detached Flag that describes if a thread should be detached one or not
	* @warnings Assertion may occuried:
	*	- if Client's socket have been already closed or haven't be opened
	*	- if a thread has already started for this Client
	*/
	template<typename Func>
	void Start(Func func, bool detached = false)
	{
		ASSERT(*m_Sock.m_SockRef != INVALID_SOCKET, "Cannot start a Client thread with a not opened Socket!");
		ASSERT(m_State == ThreadState::None, "Client has already a thread running!");
		if constexpr (std::is_same<FUNC_ARG<Func>, Socket>::value)
			m_Handle = std::thread(func, m_Sock);
		else if constexpr (std::is_same<FUNC_ARG<Func>, SOCKET>::value)
			m_Handle = std::thread(func, m_Sock.GetNativeFD());
		else { ASSERT(false, "Function signature is not correct!"); }
		if (detached)
		{
			m_State = ThreadState::Detached;
			m_Handle.detach();
			m_Sock.m_SockRef = std::make_shared<SOCKET>(INVALID_SOCKET);
		}
		else
			m_State = ThreadState::Running;
	}

	/**
	* @brief Wait for Client's thread to finish
	* @warnings Assertion may occurried if Client's thread has not being created or thread is detached.
	*/
	void Join(void) noexcept;

	/**
	* @brief Copy assignement operation
	* @details Closes the current Client and copying the context from another one to it
	* @param rhs Client that will be copied
	* @warning Generally calling copying operations on Clients should avoided look at the wiki for more info.
	*/
	BasicClient& operator=(const BasicClient& rhs) noexcept;

	/**
	* @brief Move assignement operation
	* @details Close the current Client if it is opened and moving the context from another one to it
	* @param other An r-value of a Client
	* @warning The rhs Client is becoming like a default contructed one
	*/
	BasicClient& operator=(BasicClient&& rhs) noexcept;

protected:

	/**
	* @brief Client's Socket
	*/
	Socket m_Sock;

private:

	enum class ThreadState : BYTE {
		None = 0x00,
		Running = 0x01,
		Detached = 0x02
	};

private:

	/**
	* @brief Handle to a Client's thread
	*/
	std::thread m_Handle;

	/**
	* @brief Client's thread state
	*/
	ThreadState m_State = ThreadState::None;
};

namespace TCP {

	/**
	* @brief Object that provides Client's operations
	* @details A wrapper of a Socket object to perform Clients operations
	* @author Vasileios Mormoris
	* @version 1.0.0
	* @date 2020
	* @copyright MIT
	*/
	class Client : public BasicClient {
	public:

		//Constructor(s) & Destructor
		Client(void) = default;
		Client(const Client&) = default;

		/**
		* @brief Constructs a Client and opens underlying Socket
		* @param family Adress Family that the Socket will use
		*/
		Client(int family) noexcept;

		/**
		* @brief Constructs a Client and establish a connection
		* @param family Adress Family that the Client will use
		* @param address The IP Address of the remote host
		* @param port Port number for the process of the remote host
		*/
		Client(int family, const char* address, unsigned short port) noexcept;

		/**
		* @brief Constructs a Client and establish a connection
		* @param family Adress Family that the Client will use
		* @param address A pair of IPv4 Address and Port of the remote host process
		*/
		Client(const sockaddr_in& address) noexcept;
		
		/**
		* @brief Constructs a Client and establish a connection
		* @param family Adress Family that the Client will use
		* @param address A pair of IPv6 Address and Port of the remote host process
		*/
		Client(const sockaddr_in6& address) noexcept;

		/**
		* @brief Establish connection with a remote host (aka a Client)
		* @param address The IP Address of the remote host
		* @param port Port number for the process of the remote host
		* @warnings Assertion may occurried if the address is not the same family as the one that the Client is opened with
		*/
		void Connect(const char* address, unsigned short port) noexcept;

		/**
		* @brief Establish connection with a remote host (aka a Client)
		* @param address A pair of IPv4 Address and Port of the remote host process
		* @warnings Assertion may occuried if Client's socket is opened with an Address Family other than IPv4
		*/
		void Connect(const sockaddr_in& address) noexcept;

		/**
		* @brief Establish connection with a remote host (aka a Client)
		* @param address A pair of IPv6 Address and Port of the remote host process
		* @warnings Assertion may occuried if Client's socket is opened with an Address Family other than IPv6
		*/
		void Connect(const sockaddr_in6& address) noexcept;

		/**
		* @brief Sends data to the connected remote host
		* @param data Pointer to the data that will be send
		* @param length Number of bytes that will be send
		* @param offset How many bytes away from the start should we start sending
		* @warnings Assertion may occuried if Client's socket have been already closed or haven't be opened
		*/
		int Send(const void* data, unsigned int length, unsigned int offset = 0) const noexcept;

		/**
		* @brief Sends data from the connected remote host
		* @param data Pointer to the data that will be received
		* @param length Number of bytes that will be received
		* @param offset How many bytes away from the start should we start receiving
		* @warnings Assertion may occuried if Client's socket have been already closed or haven't be opened
		*/
		int Receive(void* data, unsigned int length, unsigned int offset = 0) const noexcept;

		/**
		* @brief Disconnects the Client from an established connection
		* @warnings Assertion may occuried if Client's socket have been already closed or haven't be opened
		*/
		void Disconnect(void) noexcept;

		Client& operator=(const Client&) = default;

	private:

		friend class Server;

	};

}

namespace UDP {

	/**
	* @brief Object that provide Client's operation
	* @details A high level abstraction of the Socket operations that Clients performs
	* @author Vasileios Mormoris
	* @version 1.0.0
	* @date 2020
	* @copyright MIT
	*/
	class Client : public BasicClient{
	public:

		//Constructor(s) & Destructor
		Client(void) = default;
		Client(const Client&) = default;

		/**
		* @brief Constructs a Client and opens underlying Socket
		* @param Adress Family that the Client will use
		*/
		Client(int family) noexcept;

		/**
		* @brief Constructs a Client and binds it's underlying Socket
		* @param family Adress Family that the Client will use
		* @param address The IP Address of the Client
		* @param port Port number of the Client
		*/
		Client(int family, const char* address, unsigned short port) noexcept;

		/**
		* @brief Constructs a Client and binds it's Socket
		* @param family Adress Family that the Client will use
		* @param address A pair of IPv4 Address and Port of the Client
		*/
		Client(const sockaddr_in& address) noexcept;

		/**
		* @brief Constructs a Client and binds it's Socket
		* @param family Adress Family that the Client will use
		* @param address A pair of IPv6 Address and Port of the Client
		*/
		Client(const sockaddr_in6& address) noexcept;

		/**
		* @brief Sends data to the specified address
		* @param data Pointer to the data that will be send
		* @param address IPv4 pair of address & port
		* @param length Number of bytes that will be send
		* @param offset How many bytes away from the start should we start sending
		* @warnings Assertion may occuried if Client's socket is opened with an Address Family other than IPv4
		*/
		int SendTo(const void* data, const sockaddr_in& address, unsigned int length, unsigned int offset = 0) const noexcept;
		
		/**
		* @brief Sends data to the specified address
		* @param data Pointer to the data that will be send
		* @param address IPv6 pair of address & port
		* @param length Number of bytes that will be send
		* @param offset How many bytes away from the start should we start sending
		* @warnings Assertion may occuried if Client's socket is opened with an Address Family other than IPv6
		*/
		int SendTo(const void* data, const sockaddr_in6& address, unsigned int length, unsigned int offset = 0) const noexcept;

		/**
		* @brief Receives data
		* @param data Pointer to the data that will be received
		* @param address IPv4 pair of address & port
		* @param length Number of bytes that will be received
		* @param offset How many bytes away from the start should we start receiving
		* @warnings Assertion may occuried if Client's socket is opened with an Address Family other than IPv4
		*/
		int ReceiveFrom(void* data, sockaddr_in& address, unsigned int length, unsigned int offset = 0) const noexcept;

		/**
		* @brief Receives data
		* @param data Pointer to the data that will be received
		* @param address IPv6 pair of address & port
		* @param length Number of bytes that will be received
		* @param offset How many bytes away from the start should we start receiving
		* @warnings Assertion may occuried if Client's socket is opened with an Address Family other than IPv6
		*/
		int ReceiveFrom(void* data, sockaddr_in6& address, unsigned int length, unsigned int offset = 0) const noexcept;

		Client& operator=(const Client&) = default;

	};

}

#endif