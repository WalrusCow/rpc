#pragma once

#include <string>

struct Message {
  enum class Type {
    INVALID,
    TERMINATION,
    REGISTRATION,
    CALL,
    ADDRESS,
    SERVER_REGISTRATION,
  };

  Message() {}
  Message(Type type_, const std::string& message_)
    : type(type_), message(message_) {}

  Type type = Type::INVALID;
  std::string message;
};
