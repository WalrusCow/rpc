#pragma once

#include <string>

struct Message {
  enum class Type {
    INVALID,
    TERMINATION, // Terminate the service
    RPC_REGISTRATION, // Server registers an rpc
    CALL, // Client asking for rpc call
    ADDRESS, // Client asking for server address
    SERVER_REGISTRATION, // Register a server
    SERVER_READY, // Server is ready to handle clients
  };
  Message() {}
  Message(Type type_, const std::string& message_)
    : type(type_), message(message_) {}

  Type type = Type::INVALID;
  std::string message;
};
