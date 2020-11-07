// File Name: sockets.h
// Purpose:   To have one header file to expose berkeley sockets, smooth out the platform subtle differences and to
//            provide helper functions that help to create common TCP/IP sockets.
// Language:  C++
// Platform:  Visual Studio 15.8.2 w/ C++17, g++ (Ubuntu 7.3.0-16ubuntu3) 7.3.0
// Author:    Kim Hyungseob
// Creation date: 2018/11/30
#pragma once

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#include <WS2tcpip.h>   // almost everything is contained in one file.
#else                   // UNIX/Linux
#include <arpa/inet.h>  // contains all inet_* functions
#include <errno.h>      // contains the error functions
#include <fcntl.h>      // file control
#include <netdb.h>      // for DNS - gethostbyname()
#include <netinet/in.h> // IPv4 and IPv6 stuff
#include <sys/socket.h> // header containing socket data types and functions
#include <sys/types.h>  // header containing all basic data types and typedefs
#include <unistd.h>     // for gethostname()
#include <cstring>      // for memset
#endif


namespace sockets
{
#ifdef _WIN32

    typedef SOCKET sock; // Although sockets are int's on unix,
                         // windows uses it's own typedef of
                         // SOCKET to represent them. If you look
                         // in the Winsock2 source code, you'll see
                         // that it is just a typedef for unsigned int, but
                         // there is absolutely no guarantee that it
                         // won't change in a later version.
                         // therefore, in order to avoid confusion,
                         // this library will create it's own basic
                         // socket descriptor typedef

    // Use Window's macro for invalid socket, so that it matches
    // with their SOCKET type.
    constexpr sock BAD_SOCKET = INVALID_SOCKET;

#else // UNIX/Linux
    using sock = int;
    constexpr sock BAD_SOCKET = ~0;
#endif

    enum class Protocol
    {
        IPv4,         // To use IP addresses like 220.69.157.148
        IPv6,         // To use IP addresses like fe80::4176:ad1b:9b82:33b7%7
        HostPreferred // Use what the host machine prefers
    };

    void SendData(sock receiver, const char* buffer, int length);

    /**
     * \brief Open a listening socket, to be used by servers to accept client connection requests.
     * \param service which port service to create the socket on
     * \param protocol use this to pick which version of the IP you want the listening socket to work with.
     * \return socket with the host's ip address and the provided port value or BAD_SOCKET on failure.
     */
    sock open_listen_socket(const char* service, Protocol protocol = Protocol::HostPreferred);


    /**
     * \brief Open a client socket, to be used by clients to make a connection request and then continue to use it to
     * send/receive data from the server.
     * \param host provide a way to address the machine to connect to, which could be an ip address or domain name.
     * \param service provide the port/service associated with the process to connect to on the   machine to  connect to.
     * \return socket with the server's ip address and the provided port value or BAD_SOCKET on failure.
     */
    sock open_client_socket(const char* host, const char* service);


    /**
     * \brief Use this to close and free up the resources associated with the provided socket.
     * \param socket_handle should be a currently active socket
     */
    void close_socket(sock socket_handle);
}
