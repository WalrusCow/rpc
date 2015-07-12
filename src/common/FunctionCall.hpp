#pragma once

#include <string>

#include "common/FunctionSignature.hpp"
#include "common/Message.hpp"

class FunctionCall {
 public:
  FunctionCall(FunctionSignature&& signature_, void** data);
  FunctionCall(FunctionSignature&& signature_, const std::string& data);
  FunctionCall(FunctionCall&& o)
      : signature(std::move(o.signature)),
        dataContainer(std::move(o.dataContainer)) {
    copyArgPointers();
  }

  FunctionSignature signature;

  std::string serialize() const;
  static FunctionCall deserialize(const Message& message);

  void writeDataTo(void** out);
  void** getArgArray();

 private:
  std::vector<char> dataContainer;
  std::vector<void*> argPointers;

  void copyArgPointers();
};
