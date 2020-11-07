// File Name: echoserver.cpp
// Purpose:   Defines an echo server program.
// Language:  C++
// Platform:  Visual Studio 15.8.2 w/ C++17, g++ (Ubuntu 7.3.0-16ubuntu3) 7.3.0
// Author:    Kim Hyungseob
// Creation date: 11/28/2018

#include "sockets.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

void Get_and_store_message(sockets::sock data_socket);
void print_client_information(sockaddr_storage client_address, socklen_t socket_address_storage_size);

namespace storage
{
    std::vector<sockets::sock> browsers;
    std::string entireText;
	
	std::mutex socket_mutex;
	std::mutex text_mutex;
}

int main(int argc, char const* argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " <port or service name>\n";
        return 1;
    }

    const char* port = argv[1];

    const sockets::sock listen_socket = sockets::open_listen_socket(port, sockets::Protocol::IPv4);

    socklen_t socket_address_storage_size = sizeof(sockaddr_storage);

    while (true)
    {
        sockaddr_storage client_address = {};

        sockets::sock new_client_data_socket = accept(listen_socket, (sockaddr*)&client_address, &socket_address_storage_size);

        if (new_client_data_socket == sockets::BAD_SOCKET)
            continue; // this connection failed to happen, so let's wait for a new one

        print_client_information(client_address, socket_address_storage_size);

        std::thread client{ Get_and_store_message, new_client_data_socket};
        client.detach();
    }

    for (auto browser_socket : storage::browsers)
        sockets::close_socket(browser_socket);
    sockets::close_socket(listen_socket);
    return 0;
}

void Get_and_store_message(sockets::sock data_socket)
{
    std::array<char, 512> receive_buffer{};
    std::string nickname;

    // Checks whether client is browser or writer.
    auto bytes_received = recv(data_socket, receive_buffer.data(), static_cast<int>(receive_buffer.max_size())-1, 0);
    if (!strncmp(receive_buffer.data(), "{Browser}", bytes_received))
    {
		std::lock_guard<std::mutex> lock(storage::socket_mutex);
        storage::browsers.push_back(data_socket);
        if (send(data_socket, storage::entireText.c_str(), static_cast<int>(storage::entireText.size()), 0) <= 0)
            if (storage::entireText.size() > 0) std::cout << "Failed to send storage text." << std::endl;
        return;
    }
    else
    {
        storage::entireText += receive_buffer.data();
        std::copy(std::begin(receive_buffer)+1, std::begin(receive_buffer)+bytes_received-1, std::back_inserter(nickname));
        for (auto socket : storage::browsers)
        {
            send(socket, receive_buffer.data(), bytes_received, 0);
        }
    }

    // Message loop
    bool client_active = true;
    while (client_active)
    {
        std::string sentence;
        bytes_received = 0;

        while (true)
        {
            auto current_receive =  recv(data_socket, receive_buffer.data(), static_cast<int>(receive_buffer.max_size())-1, 0);
            
            bytes_received += current_receive;

            if (bytes_received <= 0)
            {
                nickname += '@';
                for (auto socket : storage::browsers)
                    send(socket, nickname.c_str(), nickname.size(), 0);
                client_active = false;
                break;
            }
            
            if (current_receive <= 0)
                break;

            receive_buffer[current_receive] = 0;
            sentence += receive_buffer.data();
            
            if (receive_buffer[current_receive - 1] == '}')
            {
				std::lock_guard<std::mutex> lock(storage::text_mutex);
                storage::entireText += sentence;
                if (receive_buffer[current_receive] != '{')
                    break;
				sentence.clear();
				bytes_received = 0;
            }
        }
        
        for (auto socket : storage::browsers)
        {
            sockets::SendData(socket, sentence.c_str(), bytes_received);
        }
    }
    
    sockets::close_socket(data_socket);
}

void print_client_information(sockaddr_storage client_address, socklen_t socket_address_storage_size)
{
    constexpr int NameBufferLength = 512;
    std::array<char, NameBufferLength> client_hostname{};
    std::array<char, NameBufferLength> client_port{};
    const auto psocketaddress_information = reinterpret_cast<sockaddr*>(&client_address);

    getnameinfo(psocketaddress_information, socket_address_storage_size, client_hostname.data(), NameBufferLength, client_port.data(), NameBufferLength, NI_NUMERICHOST);
    std::cout << "Connected to client (" << client_hostname.data() << ", " << client_port.data() << ") / ";

    getnameinfo(psocketaddress_information, socket_address_storage_size, client_hostname.data(), NameBufferLength, client_port.data(), NameBufferLength, NI_NUMERICSERV);
    std::cout << "(" << client_hostname.data() << ", " << client_port.data() << ")\n\n";
}