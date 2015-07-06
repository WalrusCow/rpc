#pragma once

#include <functional>
#include <string>
#include <sstream>

#include "common/Message.hpp"

class Connection {
 public:
  Connection(int socket_);
  // We need g++5.0 in order to be able to std::move a stringstream :(
  Connection(Connection&& o) :
      socket(std::move(o.socket)),
      port(std::move(o.port)),
      hostname(std::move(o.hostname)),
      bytesToRead(std::move(o.bytesToRead)),
      messageType(std::move(o.messageType)) {
    ss << o.ss;
  }

  // Return negative on error; 0 on success. Blocks.
  int send(Message::Type type, const std::string& str);

  // Return negative on error; positive on done; 0 on not done
  int read(Message::Type* type, std::string* result);
  int recv(Message::Type* type, std::string* result);

  void close();

  // TODO: Do we need this?
  //void useSocket(int socket_);

  int socket;
  int port;
  std::string hostname;

 private:
  static const uint32_t BUFFER_LEN;

  std::stringstream ss;
  uint32_t bytesToRead = 0;
  Message::Type messageType = Message::Type::INVALID;

  int doRead(Message::Type* type,
             std::string* result,
             const std::function<ssize_t(int, char*, size_t)>& reader);
};
