#include "rpc.h"
#include "client/client.hpp"

int client()  {
  return 1;
}

extern "C" int rpcInit() {
  return client();
}
