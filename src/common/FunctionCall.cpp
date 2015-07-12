#include "common/FunctionCall.hpp"

#include <cstring>
#include <sstream>

#include <iostream>

FunctionCall::FunctionCall(
    FunctionSignature&& signature_, const std::string& data)
    : signature(std::move(signature_)),
      dataContainer(data.begin(), data.end()) {
  // Check for sanity...
  std::cerr << "string: Data container: " << dataContainer.size()
            << " Signature size: " << signature.getDataSize() << std::endl;
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

  std::cerr << "voids Data container: " << dataContainer.size()
            << " Signature size: " << signature.getDataSize() << std::endl;
  copyArgPointers();
}

FunctionCall FunctionCall::deserialize(const Message& message) {
  FunctionSignature signature = FunctionSignature::deserialize(message);

  // Count trailing null bytes
  size_t bytesRead = sizeof(int) * (signature.numArgs() + 1);
  bytesRead += signature.name.size() + 1;

  std::string msgCopy = message.message;
  msgCopy.erase(0, bytesRead);

  std::cerr<<std::hex<<"Function call deserialized: ";
  for (char c : msgCopy) {
    int x = ((int)c) & 0xff;
    std::cerr << x << ' ';
  }
  std::cerr<<std::dec<<std::endl;
  return FunctionCall(std::move(signature), msgCopy);
}

std::string FunctionCall::serialize() const {
  std::stringstream ss;
  ss << signature.serialize();
  ss.write((const char*) dataContainer.data(), dataContainer.size());
  std::cerr<<"Serialized function call: ";
  std::string k = ss.str();
  std::cerr<<std::hex;
  for (char c : k) {
    int i = ((int)c)&0xff;
    std::cerr<<i<<' ';
  }
  std::cerr<<std::dec<<std::endl;
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
