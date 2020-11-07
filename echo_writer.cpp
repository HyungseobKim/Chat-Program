// File Name: echo_writer.cpp
// Purpose:   Defines a writing client that send messages to chat room.
// Language:  C++
// Platform:  Visual Studio 15.8.2 w/ C++17, g++ (Ubuntu 7.3.0-16ubuntu3) 7.3.0
// Author:    Kim Hyungseob
// Creation date: 11/30/2018

#include "sockets.h"
#include <array>
#include <iostream>
#include <string>

int main(int argc, char const* argv[])
{
    std::string nickname;

    if (argc == 4)
    {
        nickname = argv[3];
    }
    else if (argc == 3)
    {
        std::cout << "Please enter in your nickname: ";
        std::getline(std::cin, nickname);
    }
    else if (argc < 3)
    {
        std::cerr << "usage: " << argv[0] << " <server ip address> <port or service name> [nickname]\n";
        return 1;
    }

    const char* const server_host = argv[1];
    const char* const port = argv[2];

    const sockets::sock client_socket = sockets::open_client_socket(server_host, port);

    if (client_socket == sockets::BAD_SOCKET)
        return 1;

    std::string nametemp = "[" + nickname + "]";
    send(client_socket, nametemp.c_str(), static_cast<int>(nametemp.length()), 0);

    std::string input_line;
    while (true)
    {
        std::getline(std::cin, input_line);
        if (input_line.empty())
            break;

        std::string temp = "{" + nickname + "> " + input_line + "}";
        sockets::SendData(client_socket, temp.c_str(), static_cast<int>(temp.length()));
        
        if (input_line == "\\quit")
            break;
    }

    sockets::close_socket(client_socket);
    return 0;
}