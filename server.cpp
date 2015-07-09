#include "rpc.h"

int main(void) {
  if (!rpcInit())
    rpcExecute();
}
