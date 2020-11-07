// File Name: sockets.cpp
// Purpose:   To implement the platform specifics needed for berkeley sockets.
// Language:  C++
// Platform:  Visual Studio 15.8.2 w/ C++17, g++ (Ubuntu 7.3.0-16ubuntu3) 7.3.0
// Author:    Kim Hyungseob
// Creation date: 11/19/2018

#include "sockets.h"
#include <iostream>

namespace
{
    bool can_get_address_info(const char* host, const char* service, const addrinfo* hints, addrinfo** results);
}

namespace sockets
{
    void SendData(sock receiver, const char* buffer, int length)
    {
        auto bytes_send = send(receiver, buffer, length, 0);

        while (bytes_send < length)
            bytes_send += send(receiver, buffer+bytes_send, length-bytes_send, 0);
    }

    sock open_listen_socket(const char* service, Protocol protocol)
    {
        sock listen_socket{BAD_SOCKET};
        
        addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if (protocol == Protocol::IPv4)
            hints.ai_family = AF_INET;
        else if (protocol == Protocol::IPv6)
            hints.ai_family = AF_INET6;
        else
            hints.ai_flags |= AI_ADDRCONFIG;

        addrinfo* results;
        if (!can_get_address_info(NULL, service, &hints, &results)) std::cout << "get address info failed" << std::endl;
        
        // loop through all configurations and find one that we can create a socket with
        // and bind our desired (ip:port) pair to
        addrinfo* p;
        for (p = results; p; p = p->ai_next)
        {
            listen_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (listen_socket < 0)  // Because it is bad socket,
                continue; // skip this and go to next config.

            if (bind(listen_socket, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0)
                break; // If it is OK then, we done. Break.

            close_socket(listen_socket); // If it was failed to connect, close this socket.
        }

        freeaddrinfo(results); // clean up
        if (!p) std::cout << "Failed to clean up" << std::endl;

        // turn the socket into a listening socket
        // suggested backlog count is 1024

        if (listen(listen_socket, 1024) < 0)
        {
            close_socket(listen_socket);
            std::cout << "Failed to listen" << std::endl;
        }

        return listen_socket;
    }

    sock open_client_socket(const char* host, const char* service)
    {
        sock client_socket{BAD_SOCKET};
        
        addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_ADDRCONFIG;

        addrinfo* results;
        if (!can_get_address_info(host, service, &hints, &results)) std::cout << "get address info failed" << std::endl;

        // loop through the possible configurations to find one that allows us to create a socket and connect to a server
        addrinfo* p;
        for (p = results; p; p = p->ai_next)
        {
            client_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (client_socket < 0)  // Because it is bad socket,
                continue; // skip this and go to next config.
            
            if (connect(client_socket, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0)
                break; // If it is OK then, we done. Break.

            close_socket(client_socket); // If it was failed to connect, close this socket.
        }

        freeaddrinfo(results); // clean up
        if (!p) std::cout << "Failed to clean up" << std::endl;

        return client_socket;
    }

    void close_socket(sock socket_handle)
    {
#ifdef _WIN32
        if (closesocket(socket_handle))
        {
            int err = WSAGetLastError();
            std::cout << "Failed to close socket. Err: " << err << std::endl;
        }
            
#else
	if (close(socket_handle))
            std::cout << "Failed to close socket. Err: " << std::endl;
#endif
    }
}
namespace
{
    bool can_get_address_info(const char* host, const char* service, const addrinfo* hints, addrinfo** results)
    {
        auto err = getaddrinfo(host, service, hints, results);

        if (err)
        {
            std::cout << gai_strerror(err);
            return false;
        }

        return true;
    }
// ========================================================================
//  This class is designed to be a global singleton that initializes
//  and shuts down Winsock.
// ========================================================================
#ifdef _WIN32
    class WindowsSocketSystem
    {
    public:
        WindowsSocketSystem()
        {
            // TODO Startup WinSock version 2.2
            // https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-wsastartup
            WORD wVersionRequested = MAKEWORD(2, 2);
            WSADATA wsaData;

            auto err = WSAStartup(wVersionRequested, &wsaData);
            if (err != 0)
            {
                std::cout << "Failed to WSAStartup. Error:" << err;
                return;
            }

            if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
            {
                std::cout << "Failed find a usable version of Winsock.dll";
                WSACleanup();
                return;
            }
        }
        ~WindowsSocketSystem()
        {
            // TODO Cleanup WinSock
            // https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-wsacleanup
            WSACleanup();
        }
    };
    WindowsSocketSystem g_system;
#endif
}