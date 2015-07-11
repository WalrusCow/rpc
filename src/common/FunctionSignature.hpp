#pragma once

#include <string>
#include <vector>

#include "common/Message.hpp"

class FunctionSignature {
 public:
  FunctionSignature(const std::string& name_, const int* argTypes_);
  FunctionSignature(std::string&& name_, std::vector<int>&& argTypes_);

  std::string name;
  std::vector<int> argTypes;

  static FunctionSignature deserialize(const Message& message);
  std::string serialize() const;

  bool operator==(const FunctionSignature& other) const;

  size_t getDataSize() const;
  size_t getArgSize(size_t idx) const;
  size_t numArgs() const;

  int* getArgTypes();

 private:
  // Lower bits are for the array size
  static const int ARR_BITS;
  static const int TYPE_BITS;
  size_t getTypeSize(int type) const;
};
