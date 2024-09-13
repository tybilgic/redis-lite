#include <thread>

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
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(m_server_socket, (sockaddr *)&client_addr, &client_len);

        if (client_socket < 0)
        {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }

        std::thread(&Server::handle_client, this, client_socket).detach();
    }
}

void Server::handle_client(int client_socket)
{
    char buffer[1024] = {0};

    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0)
    {
        std::cerr << "Failed to read from client" << std::endl;
        close(client_socket);
        return;
    }

    std::string request(buffer);
    std::cout << "Received: " << request << std::endl;

    std::string response = "+PONG\r\n";
    send(client_socket, response.c_str(), response.size(), 0);

    close(client_socket);
}