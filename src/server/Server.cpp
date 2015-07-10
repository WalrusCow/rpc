#include "server/Server.hpp"

#include <iostream>
#include <unistd.h>

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
  return binderConnection->send(Message::Type::SERVER_REGISTRATION, "") == 0;
}

bool Server::connect() {
  return connectToBinder() && server.connect(&serverAddress);
}

bool Server::run() {
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
  for (const auto& function : registeredFunctions) {
    if (function.signature == signature) {
      warning = true;
      break;
    }
  }

  registeredFunctions.emplace_back(signature, f);

  // Now send to binder
  if (!binderConnection->send(
      Message::Type::RPC_REGISTRATION, signature.serialize())) {
    // Error
    return -1;
  }

  return warning ? 1 : 0;
}

bool Server::handleMessage(const Message& message, Connection& conn) {
  switch (message.type) {
  case Message::Type::CALL:
    // Call shit now
    break;

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
    server.stop();
    return true;
  }
  else {
    conn.close();
    server.stop();
    return false;
  }
}
