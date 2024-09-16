#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

#include "Server.hpp"

std::atomic<bool> running(true);

void signal_handler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        running = false;
        std::cout << "\nShutting down the server..." << std::endl;
    }
}

int main(int argc, char *argv[])
{
    int port = 6379; // default redis port?

    if (argc > 1)
    {
        try
        {
            port = std::stoi(argv[1]);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Invalid port number provided. Using default port 6379" << std::endl;
            port = 6379;
        }
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        Server server(port);
        std::thread server_thread([&server]()
                                  { server.start(); });

        while (running)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Server is running..." << std::endl;
        }

        server.stop();
        server_thread.join();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}