#include "common/ServerAddress.hpp"

#include <sstream>

ServerAddress ServerAddress::deserialize(const Message& message) {
  // first the port, then the rest of the name
  int port = 0;
  for (size_t i = 0; i < sizeof(int); ++i) {
    int x = ((int) message.message[i]) & 0xff;
    port |= (x << (8 * i));
  }

  // Remaining message is the hostname
  std::string hostname = (message.message.c_str() + sizeof(int));
  return ServerAddress(port, std::move(hostname));
}

std::string ServerAddress::serialize() const {
  std::stringstream ss;
  ss.write((const char*)&port, sizeof(int));
  ss << hostname;
  return ss.str();
}
