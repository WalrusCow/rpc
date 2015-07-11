#include "server/Server.hpp"

#include <iostream>
#include <unistd.h>

#include "common/FunctionCall.hpp"

bool Server::connectToBinder() {
  char* binderHostname = getenv("BINDER_ADDRESS");
  if (!binderHostname) {
    std::cerr << "Could not get BINDER_ADDRESS from environment!" << std::endl;
    return false;
  }
  char* portStr = getenv("BINDER_PORT");
  if (!portStr) {
    std::cerr << "Could not get BINDER_PORT from environment!" << std::endl;
    return false;
  }

  // Binder information
  int binderPort = std::stoi(portStr);
  binderServer = gethostbyname(binderHostname);
  binderAddr.sin_family = AF_INET;
  bcopy((char*)binderServer->h_addr,
        (char*)&binderAddr.sin_addr.s_addr,
        (int)binderServer->h_length);
  binderAddr.sin_port = htons(binderPort);

  // Now connect to binder socket
  binderSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (binderSocket < 0) {
    return false;
  }
  if (::connect(
        binderSocket, (struct sockaddr*)&binderAddr, sizeof(binderAddr)) < 0) {
    return false;
  }
  binderClientList.clients.emplace_back(binderSocket);
  binderConnection = &binderClientList.clients.front();
  int success = binderConnection->send(
      Message::Type::SERVER_REGISTRATION,
      serverAddress.serialize());
  return success == 0;
}

bool Server::connect() {
  return server.connect(&serverAddress) && connectToBinder();
}

bool Server::run() {
  if (registeredFunctions.size() == 0) {
    // Did not register any functions: Error
    return false;
  }

  server.addClientList(&clients);
  server.addClientList(&binderClientList);
  if (binderConnection->send(Message::Type::SERVER_READY, "") < 0) {
    return false;
  }

  // TODO: Join message handling threads
  return server.serve();
}

int Server::registerRpc(const std::string& name, int* argTypes, skeleton f) {
  bool warning = false;
  FunctionSignature signature(name, argTypes);
  // Used if we have the same signature as an existing function (overwrite)
  ServerFunction* oldFunction = nullptr;
  for (auto& function : registeredFunctions) {
    if (function.signature == signature) {
      warning = true;
      oldFunction = &function;
      break;
    }
  }

  // Now send to binder
  if (!binderConnection->send(
      Message::Type::RPC_REGISTRATION, signature.serialize())) {
    // Error
    return -1;
  }

  if (oldFunction) {
    *oldFunction = ServerFunction(signature, f);
  }
  else {
    registeredFunctions.emplace_back(signature, f);
  }

  return warning ? 1 : 0;
}

bool Server::handleMessage(const Message& message, Connection& conn) {
  switch (message.type) {
  case Message::Type::CALL:
  {
    // TODO: Put this in a damn function
    // Call me maybe...
    auto functionCall = FunctionCall::deserialize(message);
    ServerFunction* function = getFunction(functionCall.signature);
    if (function == nullptr) {
      conn.close();
      break;
    }
    int* argTypes = functionCall.signature.getArgTypes();
    void** args = functionCall.getArgArray();
    function->function(argTypes, args);
    conn.send(Message::Type::CALL, functionCall.serialize());
    break;
  }

  default:
    // Some invalid type
    conn.close();
    break;
  }
  // Always remove it from the list
  return true;
}

bool Server::handleBinderMessage(const Message& message, Connection& conn) {
  if (message.type == Message::Type::TERMINATION) {
    // terminate
    // TODO : May need thread cleanup, etc
    conn.close();
    server.stop();
    return true;
  }
  else {
    conn.close();
    server.stop();
    return false;
  }
}

Server::ServerFunction* Server::getFunction(
    const FunctionSignature& signature) {
  for (auto& function : registeredFunctions) {
    if (function.signature == signature) {
      return &function;
    }
  }
  return nullptr;
}
