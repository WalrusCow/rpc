#pragma once

#include <string>

#include "common/FunctionSignature.hpp"
#include "common/Message.hpp"

class FunctionCall {
 public:
  FunctionCall(const FunctionSignature& signature_, void* data_)
      : signature(signature_), data(data_) {}
  FunctionCall(FunctionSignature&& signature_, void* data_)
      : signature(std::move(signature_)), data(data_) {}

  FunctionCall(FunctionSignature&& signature_, const std::string& data_);

  FunctionSignature signature;
  void* data;

  std::string serialize();
  static FunctionCall deserialize(const Message& message);

 private:
  std::vector<char> dataContainer;
};
