#include "binder/ServerConnection.hpp"

#include <iostream>

void ServerConnection::terminate() {
  // We must write over the socket
  // Server will add binder connection to read set
  // All messages are of form SIZE|TYPE|MESSAGE
  // int send(uint32_t type, uint32_t message) or send(message)
  // int read(uint32_t* type, std::string* message)
  auto code = send(MessageType::TERMINATION, "");
  if (code < 0) {
    // Error in terminating
    std::cerr << "Error in sending termination message to server." << std::endl;
  }
  close();
}
