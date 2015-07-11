#pragma once

#include <string>

#include "common/Message.hpp"

class ServerAddress {
 public:
  ServerAddress() {}
  ServerAddress(int port_, const std::string& hostname_)
      : port(port_), hostname(hostname_) {}
  ServerAddress(int port_, std::string&& hostname_)
      : port(port_), hostname(std::move(hostname_)) {}

  int port;
  std::string hostname;

  std::string serialize() const;
  static ServerAddress deserialize(const Message& message);
};
