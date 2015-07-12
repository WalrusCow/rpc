#include "common/FunctionCall.hpp"

#include <cstring>
#include <sstream>

FunctionCall::FunctionCall(
    FunctionSignature&& signature_, const std::string& data)
    : signature(std::move(signature_)),
      dataContainer(data.begin(), data.end()) {
  // Check for sanity...
  copyArgPointers();
}

FunctionCall::FunctionCall(FunctionSignature&& signature_, void** data)
    : signature(std::move(signature_)) {
  // Copy data in from data_
  for (size_t i = 0; i < signature.numArgs(); ++i) {
    // Byte pointer
    char* bytes = (char*) data[i];
    // How many bytes is this current argument?
    size_t sz = signature.getArgSize(i);
    for (size_t j = 0; j < sz; ++j) {
      // Push each byte to our container
      dataContainer.push_back(bytes[j]);
    }
  }

  copyArgPointers();
}

FunctionCall FunctionCall::deserialize(const Message& message) {
  FunctionSignature signature = FunctionSignature::deserialize(message);

  // Count trailing null bytes
  size_t bytesRead = sizeof(int) * (signature.numArgs() + 1);
  bytesRead += signature.name.size() + 1;

  std::string msgCopy = message.message;
  msgCopy.erase(0, bytesRead);

  return FunctionCall(std::move(signature), msgCopy);
}

std::string FunctionCall::serialize() const {
  std::stringstream ss;
  ss << signature.serialize();
  ss.write((const char*) dataContainer.data(), dataContainer.size());
  return ss.str();
}

void FunctionCall::writeDataTo(void** out) {
  size_t sizeSoFar = 0;

  for (size_t i = 0; i < signature.numArgs(); ++i) {
    // Amount to copy
    size_t sz = signature.getArgSize(i);
    std::memcpy(out[i], (void*) (dataContainer.data() + sizeSoFar), sz);
    sizeSoFar += sz;
  }
}

void** FunctionCall::getArgArray() {
  return argPointers.data();
}

void FunctionCall::copyArgPointers() {
  char* dataPoint = dataContainer.data();
  for (size_t i = 0; i < signature.numArgs(); ++i) {
    argPointers.push_back((void*) dataPoint);
    dataPoint += signature.getArgSize(i);
  }
}
