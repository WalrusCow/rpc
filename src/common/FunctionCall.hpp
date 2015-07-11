#pragma once

#include <string>

#include "common/FunctionSignature.hpp"
#include "common/Message.hpp"

class FunctionCall {
 public:
  //FunctionCall(const FunctionSignature& signature_, void** data_);
      //: signature(signature_), data(data_) {}
  FunctionCall(FunctionSignature&& signature_, void** data);
      //: signature(std::move(signature_)), data(data_) {}

  FunctionCall(FunctionSignature&& signature_, const std::string& data);

  FunctionSignature signature;

  std::string serialize() const;
  static FunctionCall deserialize(const Message& message);

  void writeDataTo(void** out);

 private:
  std::vector<char> dataContainer;
};
