#include "common/FunctionCall.hpp"

#include <sstream>

FunctionCall::FunctionCall(
    FunctionSignature&& signature_, const std::string& data_)
    : signature(std::move(signature_)),
      dataContainer(data_.begin(), data_.end()) {
  // And keep this ghetto pointer around for ease of use
  data = (void*) dataContainer.data();
}

FunctionCall FunctionCall::deserialize(const Message& message) {
  // Deserialize ...
  // First deserialize the message
  // TODO: How many bytes were read? lol
  FunctionSignature signature = FunctionSignature::deserialize(message);

  // Count trailing null bytes
  size_t bytesRead = sizeof(int) * (signature.argTypes.size() + 1);
  bytesRead += signature.name.size() + 1;

  std::string msgCopy = message.message;
  msgCopy.erase(0, bytesRead);

  return FunctionCall(std::move(signature), msgCopy);
}

std::string FunctionCall::serialize() {
  std::stringstream ss;
  ss.write((char*)data, signature.getDataSize());
  return ss.str();
}
