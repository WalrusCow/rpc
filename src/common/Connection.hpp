#pragma once

#include <functional>
#include <string>
#include <sstream>

#include "common/MessageType.hpp"

class Connection {
 public:
  Connection(int socket_);

  // Return negative on error; 0 on success. Blocks.
  int send(MessageType type, const std::string& str);
  // Return negative on error; positive on done; 0 on not done
  int read(MessageType* type, std::string* str);

  // Return negative on error; positive on done; 0 on not done
  int read(MessageType* type, std::string* result);
  int recv(MessageType* type, std::string* result);

  void close();

  // TODO: Do we need this?
  //void useSocket(int socket_);

  int socket;
  int port;
  std::string hostname;

 private:
  std::stringstream ss;
  uint32_t bytesToRead = 0;
  MessageType messageType = 0;
  const uint32_t BUFFER_LEN = 1024;

  int doRead(MessageType* type,
             std::string* result,
             const std::function<ssize_t(int, char*, size_t)>& reader);
};
