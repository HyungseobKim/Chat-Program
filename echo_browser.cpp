// File Name: echo_browser.cpp
// Purpose:   Defines a browser client that receiveing messages and printing.
// Language:  C++
// Platform:  Visual Studio 15.8.2 w/ C++17, g++ (Ubuntu 7.3.0-16ubuntu3) 7.3.0
// Author:    Kim Hyungseob
// Creation date: 11/30/2018

#include "sockets.h"
#include <array>
#include <iostream>
#include <string>
#include <algorithm>



int main(int argc, char const* argv[])
{
    if (argc != 3)
    {
        std::cerr << "usage: " << argv[0] << " <server ip address> <port or service name>\n";
        return 1;
    }

    const char* const server_host = argv[1];
    const char* const port = argv[2];

    const sockets::sock browser_socket = sockets::open_client_socket(server_host, port);

    if (browser_socket == sockets::BAD_SOCKET)
        return 1;

    std::string browser{ "{Browser}" };
    if (send(browser_socket, browser.c_str(), browser.size(), 0) <= 0)
    {
        std::cout << "Failed to send <Browser>";
        return 1;
    }

    std::array<char, 512> receive_buffer{};
    bool client_active = true;
    while (client_active)
    {
        std::string sentence;
        int bytes_received = 0;

        while (true)
        {
            int current_receive = recv(browser_socket, receive_buffer.data(), static_cast<int>(receive_buffer.max_size())-1, 0);
            
            bytes_received += current_receive;

            if (bytes_received <= 0)
            {
                client_active = false;
                break;
            }
            if (current_receive <= 0)
            {
                break;
            }

            receive_buffer[current_receive] = 0;
            sentence += receive_buffer.data();

            if (receive_buffer[current_receive - 1] == '}' ||
                receive_buffer[current_receive - 1] == ']' ||
                receive_buffer[current_receive - 1] == '@')
                break;
        }

        if (bytes_received <= 0)
            break;
        
        for (int i = 0; i < bytes_received; ++i)
        {
            auto c = sentence[i];

            if (c == '{' || c == '[')
                continue;
            else if (c == '}')
            {
                std::cout << std::endl;
                continue;
            }
            else if (c == ']')
            {
                std::cout << " has joined to the chat room!" << std::endl;
                continue;
            }
            else if (c == '@')
            {
                std::cout << " has left chat room!" << std::endl;
                continue;
            }
            std::cout << c;
        }
    }

    sockets::close_socket(browser_socket);
    return 0;
}