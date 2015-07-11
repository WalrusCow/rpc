#include <iostream>
#include "rpc.h"

int main(void) {
  int count0 = 3;
  int argTypes0[count0 + 1];
  argTypes0[0] = (1 << ARG_OUTPUT) | (ARG_INT << 16);
  argTypes0[1] = (1 << ARG_INPUT) | (ARG_INT << 16);
  argTypes0[2] = (1 << ARG_INPUT) | (ARG_INT << 16);
  argTypes0[3] = 0;

  void** args0 = (void **)malloc(count0 * sizeof(void *));
  int a0 = 5;
  int b0 = 10;
  int return0;
  args0[0] = (void *)&return0;
  args0[1] = (void *)&a0;
  args0[2] = (void *)&b0;

  std::cerr<<  rpcCall("f0", argTypes0, args0)<<std::endl;
}
