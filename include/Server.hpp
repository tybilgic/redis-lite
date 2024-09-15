#pragma once

#include <atomic>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class Server
{
public:
    Server(int port);
    void start();
    void stop();

private:
    void handle_client(int client_socket);
    int m_server_socket{-1};
    int m_port;
    std::atomic<bool> m_shutdown;
};