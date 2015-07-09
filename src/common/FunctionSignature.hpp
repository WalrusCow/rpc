#pragma once

#include <string>
#include <vector>

#include "common/Message.hpp"

class FunctionSignature {
 public:
  FunctionSignature(const std::string& name_, const int* argTypes_);
  FunctionSignature(std::string&& name_, std::vector<int>&& argTypes_);

  static FunctionSignature deserialize(const Message& message);
  std::string serialize();

  bool operator==(const FunctionSignature& other);

 private:
  // Lower bits are for the array size
  const uint32_t ARR_BITS = 0x0000FFFF;
  const std::string name;
  std::vector<int> argTypes;
};
