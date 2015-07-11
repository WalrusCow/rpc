#pragma once

#include <list>

#include "common/FunctionSignature.hpp"
#include "common/ServerAddress.hpp"

class Server {
 public:
  Server(int socket_, ServerAddress&& address_)
      : socket(socket_), address(std::move(address_)) {}
  int socket;
  ServerAddress address;
  bool ready = false; // Is it ready to serve requests?
  std::list<FunctionSignature> signatures;
};
