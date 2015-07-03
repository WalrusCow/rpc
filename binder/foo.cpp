#include "rpc.h"
#include "binder/foo.hpp"

extern "C" int rpcInit() {
  return foo();
}

int foo() {
  return 2;
}
