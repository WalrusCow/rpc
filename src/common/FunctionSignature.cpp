#include "common/FunctionSignature.hpp"

#include <sstream>

const uint32_t FunctionSignature::ARR_BITS = 0x0000FFFF;

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

std::string FunctionSignature::serialize() {
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
