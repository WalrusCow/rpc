#pragma once

#include <list>

#include "common/Connection.hpp"
#include "common/FunctionSignature.hpp"

class Server {
 public:
  Server(Connection&& connection_) : connection(std::move(connection_)) {}
  std::list<FunctionSignature> functions;
  Connection connection;

  void terminate();
};
