#include "rpc.h"

#include "server/Server.hpp"

namespace {
  Server server;
} // Anonymous

extern "C" int rpcInit() {
  // Open connections, but do not listen yet
  return server.connect() ? 0 : -1;
}

extern "C" int rpcRegister(char* name, int* argTypes, skeleton f) {
  return server.registerRpc(name, argTypes, f) ? 0 : -1;
}

extern "C" int rpcExecute() {
  return server.run() ? 0 : -1;
}
