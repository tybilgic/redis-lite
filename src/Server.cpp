#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Server.hpp"
#include "RESPParser.hpp"

#define SOCK_INVALID -1

Server::Server(int port) : m_port(port), m_shutdown(false)
{
    m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_socket == SOCK_INVALID)
    {
        throw std::runtime_error("Failed to create server socket.");
    }

    int opt = 1;
    if (setsockopt(m_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw std::runtime_error("Failed to set socket options.");
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
    std::cout << "Server started on port " << m_port << std::endl;

    fd_set read_fds;
    timeval timeout;

    while (!m_shutdown)
    {
        FD_ZERO(&read_fds);
        FD_SET(m_server_socket, &read_fds);
        int max_fd = m_server_socket;

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int res = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);

        if (res < 0 && errno != EINTR)
        {
            std::cerr << "Select error" << std::endl;
            break;
        }

        if (res > 0)
        {
            if (FD_ISSET(m_server_socket, &read_fds))
            {
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                int client_socket = accept(m_server_socket, (sockaddr *)&client_addr, &client_len);

                if (client_socket >= 0)
                {
                    std::thread(&Server::handle_client, this, client_socket).detach();
                }
                else
                {
                    std::cerr << "Failed to accept client connection" << std::endl;
                }
            }
        }
    }

    close(m_server_socket);
    std::cout << "Server thread stopped." << std::endl;
}

void Server::stop()
{
    m_shutdown = true;
}

void Server::handle_client(int client_socket)
{
    try
    {
        char buffer[1024] = {0};
        std::string data_buffer;
        ssize_t bytes_received;

        while (true)
        {
            bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

            if (bytes_received > 0)
            {
                data_buffer.append(buffer, bytes_received);

                RESPParser parser(data_buffer);

                while (parser.has_next())
                {
                    auto command = parser.next_command();
                    process_command(client_socket, command);
                }

                // remove the processed part from the buffer
                data_buffer = parser.get_remaining_data();
            }
            else if (bytes_received == 0)
            {
                // client disconnected
                std::cout << "Client disconnected." << std::endl; // TODO: log client address?
                break;
            }
            else
            {
                // err
                std::cerr << "Recv failed." << std::endl; // TODO: log errno
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in handle_client: " << e.what() << std::endl;
    }

    close(client_socket);
}

void Server::process_command(int client_socket, const std::vector<std::string> &command)
{
    if (command.empty())
    {
        return;
    }

    std::string cmd = command[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    std::string response;

    if (cmd == "PING")
    {
        if (command.size() == 1)
        {
            response = "+PONG\r\n";
        }
        else if (command.size() == 2)
        {
            response = "+" + command[1] + "\r\n";
        }
        else
        {
            response = "-ERR wrong number of arguments for 'PING' command\r\n";
        }
    }
    else if (cmd == "ECHO")
    {
        if (command.size() == 2)
        {
            response = "+" + command[1] + "\r\n";
        }
        else
        {
            response = "-ERR wrong number of arguments for 'ECHO' command\r\n";
        }
    }
    else if (cmd == "SET")
    {
        if (command.size() >= 3)
        {
            // Handle optional expiry parameters
            std::optional<std::chrono::milliseconds> expire_time = std::nullopt;
            if (command.size() > 3)
            {
                // Process options
                for (size_t i = 3; i < command.size(); i += 2)
                {
                    std::string option = command[i];
                    if (option == "EX" && i + 1 < command.size())
                    {
                        int seconds = std::stoi(command[i + 1]);
                        expire_time = std::chrono::seconds(seconds);
                    }
                    else if (option == "PX" && i + 1 < command.size())
                    {
                        int milliseconds = std::stoi(command[i + 1]);
                        expire_time = std::chrono::milliseconds(milliseconds);
                    }
                    else
                    {
                        response = "-ERR syntax error\r\n";
                        break;
                    }
                }
            }
            if (response.empty())
            {
                m_data_store.set(command[1], command[2], expire_time);
                response = "+OK\r\n";
            }
        }
        else
        {
            response = "-ERR wrong number of arguments for 'SET' command\r\n";
        }
    }
    else if (cmd == "GET")
    {
        if (command.size() == 2)
        {
            std::string value = m_data_store.get(command[1]);
            if (!value.empty())
            {
                response = "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
            }
            else
            {
                response = "$-1\r\n"; // null bulk reply
            }
        }
        else
        {
            response = "-ERR wrong number of arguments for 'GET' command\r\n";
        }
    }
    else if (cmd == "EXISTS")
    {
        if (command.size() == 2)
        {
            bool exists = m_data_store.exists(command[1]);
            response = ":" + std::to_string(exists ? 1 : 0) + "\r\n";
        }
        else
        {
            response = "-ERR wrong number of arguments for 'EXISTS' command\r\n";
        }
    }
    else if (cmd == "DEL")
    {
        if (command.size() >= 2)
        {
            int count = 0;
            for (size_t i = 1; i < command.size(); i++)
            {
                if (m_data_store.del(command[i]))
                {
                    count++;
                }
            }
            response = ":" + std::to_string(count) + "\r\n";
        }
        else
        {
            response = "-ERR wrong number of arguments for 'DEL' command\r\n";
        }
    }
    else if (cmd == "INCR")
    {
        if (command.size() == 2)
        {
            try
            {
                int value = m_data_store.incr(command[1]);
                response = ":" + std::to_string(value) + "\r\n";
            }
            catch (const std::exception &e)
            {
                response = "-ERR " + std::string(e.what()) + "\r\n";
            }
        }
        else
        {
            response = "-ERR wrong number of arguments for 'INCR' command\r\n";
        }
    }
    else if (cmd == "DECR")
    {
        if (command.size() == 2)
        {
            try
            {
                int value = m_data_store.decr(command[1]);
                response = ":" + std::to_string(value) + "\r\n";
            }
            catch (const std::exception &e)
            {
                response = "-ERR " + std::string(e.what()) + "\r\n";
            }
        }
        else
        {
            response = "-ERR wrong number of arguments for 'DECR' command\r\n";
        }
    }
    else
    {
        response = "-ERR unknown command '" + command[0] + "'\r\n";
    }

    send(client_socket, response.c_str(), response.size(), 0);
}
