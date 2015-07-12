#pragma once

#include <string>

#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "common/Connection.hpp"
#include "common/FunctionSignature.hpp"

class Client {
 public:
  Client();
  int rpcCall(const std::string& fun, int* argTypes, void** args) const;
  int terminate() const;

 private:
  std::string binderHost;
  int binderPort;

  int connectTo(const std::string& host, int port) const;
  int connectToServer(const FunctionSignature& signature) const;
};
