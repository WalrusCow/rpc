#include "rpc.h"

#include "client/Client.hpp"

namespace {
  Client client;
} // Anonymous

extern "C" int rpcCall(char* name, int* argTypes, void** args) {
  return client.rpcCall(name, argTypes, args);
}

extern "C" int rpcTerminate() {
  return client.terminate();
}
