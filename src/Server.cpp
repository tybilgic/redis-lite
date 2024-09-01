#include "Server.hpp"

Server::Server(int port) {
  m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_server_socket == -1) {
  }
}