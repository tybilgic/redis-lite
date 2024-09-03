#include "Server.hpp"

#define SOCK_INVALID -1

Server::Server(int port) : m_port(port)
{
    m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_socket == SOCK_INVALID)
    {
        throw std::runtime_error("Failed to create server socket.");
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(m_port);

    if (bind(m_server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::string err("Failed to bind to port=" + m_port);
        throw std::runtime_error(err);
    }

    if (listen(m_server_socket, SOMAXCONN) < 0)
    {
        throw std::runtime_error("Failed to listen on socket");
    }
}

void Server::start(void)
{
    std::cout << "Server started on port" << m_port << std::endl;

    while (!m_shutdown)
    {
        sockaddr_in client_addr{};

        socklen_t client_len
    }
    
}