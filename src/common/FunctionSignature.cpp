#include "common/FunctionSignature.hpp"

#include <sstream>

#include "rpc.h"

const int FunctionSignature::ARR_BITS = 0x0000FFFF;
const int FunctionSignature::TYPE_BITS = 0x00FF0000;

FunctionSignature::FunctionSignature(
    const std::string& name_, const int* argTypes_) : name(name_) {
  while (*argTypes_) {
    argTypes.push_back(*argTypes_++);
  }
}

FunctionSignature::FunctionSignature(
    std::string&& name_, std::vector<int>&& argTypes_) :
    name(std::move(name_)), argTypes(std::move(argTypes_)) {
}

FunctionSignature FunctionSignature::deserialize(const Message& message) {
  // Deserialize from a message
  // okay.
  std::stringstream ss;
  const char* msg = message.message.c_str();
  while (*msg) {
    ss << *msg++;
  }
  msg++; // Skip \0
  std::string name(ss.str());

  std::vector<int> argTypes;
  // Treat as an int for argTypes
  const int* typePointer = (const int*)msg;
  while (*typePointer) {
    argTypes.push_back(*typePointer++);
  }

  return FunctionSignature(std::move(name), std::move(argTypes));
}

std::string FunctionSignature::serialize() const {
  std::stringstream ss;
  ss << name;
  ss << '\0';
  for (int type : argTypes) {
    ss.write((char*) &type, sizeof(type));
  }
  return ss.str();
}

bool FunctionSignature::operator==(const FunctionSignature& other) const {
  if (name != other.name || argTypes.size() != other.argTypes.size()) {
    return false;
  }

  for (size_t i = 0; i < argTypes.size(); ++i) {
    if ((argTypes[i] & ~ARR_BITS) != (other.argTypes[i] & ~ARR_BITS)) {
      return false;
    }
  }
  return true;
}

size_t FunctionSignature::getDataSize() const {
  size_t dataSize = 0;
  for (int type : argTypes) {
    size_t arrSize = type & ARR_BITS;
    if (arrSize == 0) arrSize = 1; // Not an array: one value
    dataSize += getTypeSize(type) * arrSize;
  }
  return dataSize;
}

size_t FunctionSignature::getArgSize(size_t idx) const {
  return getTypeSize(argTypes[idx]);
}

size_t FunctionSignature::numArgs() const {
  return argTypes.size();
}


size_t FunctionSignature::getTypeSize(int type) const {
  switch ((type & TYPE_BITS) >> 16) {
  case ARG_CHAR:
    return sizeof(char);
  case ARG_SHORT:
    return sizeof(short);
  case ARG_INT:
    return sizeof(int);
  case ARG_LONG:
    return sizeof(long);
  case ARG_DOUBLE:
    return sizeof(double);
  case ARG_FLOAT:
    return sizeof(float);
  default:
    return 0;
  }
}

int* FunctionSignature::getArgTypes() {
  return argTypes.data();
}
