#pragma once

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

private:
    void handle_client(int client_socket);
    int m_server_socket{-1};
    int m_port;
    bool m_shutdown{false};
};