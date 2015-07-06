#pragma once

#include "common/Connection.hpp"

class ServerConnection : public Connection {
 public:
  void terminate();
};
