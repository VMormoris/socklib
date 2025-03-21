#pragma once

//Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#define PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix)
	#define PLATFORM_UNIX
#else
	/* Not supported Platform */
	#error "Platform not Supported!"
#endif
//End of platform detection

//Assertion macro definitions
#ifdef DEBUG_BUILD
	#ifdef PLATFORM_WINDOWS//Windows
		#define DEBUG_BREAK __debugbreak()
	#else//Unix-Like
		#include <csignal>
		#define DEBUG_BREAK raise(SIGTRAP)
	#endif
	#include <cstdio>
	#define SOCKLIB_ASSERT(x, msg) {if(!(x)){fprintf(stderr, "[File: %s, line: %d] %s\n", __FILE__, __LINE__, msg); DEBUG_BREAK;}}
#else
	#define SOCKLIB_ASSERT(x, msg)
#endif
//End of assertion macro definitions

#ifdef PLATFORM_WINDOWS//Windows
	#include <WinSock2.h>
	#include <WS2tcpip.h>
	// Need to link with Ws2_32.lib
	#pragma comment(lib, "Ws2_32.lib")
	//Extra macro definition
	#define SHUT_WR SD_SEND
	#define SHUT_RD SD_RECEIVE
	#define SHUT_RDWR SD_BOTH
	namespace socklib {
		typedef int socklen_t;
		using IOSize = int;
	}
#else//Unix-Like
	#include <sys/select.h> 
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/ioctl.h>
	//Extra macro definition
	#define INVALID_SOCKET (-1)
	#define SOCKET_ERROR (-1)
	#define closesocket close
	//Extra type definition
	namespace socklib {
		using SOCKET = int;
		using IOSize = ssize_t;
	}
#endif

//Windows processes that using sockets are supposed
//	to call WSAStartup() before they start using them
//	and WSACleanup() when they are done using them
#ifdef SOCK_MAIN
	#ifdef PLATFORM_WINDOWS
	namespace socklib { extern int socklib_main(int argc, char** argv); }
	int main(int argc, char** argv)
	{
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		SOCKLIB_ASSERT(iResult == NO_ERROR, "WSAStartup Failed!");

		int result = socklib::socklib_main(argc, argv);

		WSACleanup();
		return result;
	}
	#define main socklib::socklib_main
	#endif
#endif
