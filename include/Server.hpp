#pragma once

#include <atomic>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "DataStore.hpp"

class Server
{
public:
    Server(int port);
    void start();
    void stop();

private:
    void handle_client(int client_socket);
    void process_command(int client_socket, const std::vector<std::string> &command);

    int m_server_socket{-1};
    int m_port;
    std::atomic<bool> m_shutdown;
    DataStore m_data_store;
};