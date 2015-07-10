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

  bool operator==(const FunctionSignature& other) const;

 public:
  // Lower bits are for the array size
  static const uint32_t ARR_BITS;
  std::string name;
  std::vector<int> argTypes;
};
