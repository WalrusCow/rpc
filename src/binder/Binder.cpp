#include "binder/Binder.hpp"

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <sstream>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace {

void fatalError(const std::string& error, int exitCode) {
  std::cerr << error << std::endl;
  std::exit(exitCode);
}

} // Anonymous

void Binder::connect() {
  // Connect to port
  mainSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (mainSocket == -1) {
    fatalError("Failed to create socket", -1);
  }

  char hostnameBuffer[1024];
  gethostname(hostnameBuffer, sizeof(hostnameBuffer));
  hostnameBuffer[sizeof(hostnameBuffer) - 1] = '\0';
  std::cout << "BINDER_ADDRESS " << hostnameBuffer << std::endl;

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = 0; // Next available

  if (bind(mainSocket, (struct sockaddr*) (&sin), sinLen) == -1) {
    fatalError("Failed to bind socket", -2);
  }

  // Listen with max 5 pending connections
  if (listen(mainSocket, 5) == -1) {
    fatalError("Failed to listen on socket", -3);
  }

  // Get port number
  if (getsockname(mainSocket, (struct sockaddr*) (&sin), &sinLen) == -1) {
    fatalError("Could not get socket name", -4);
  }

  std::cout << "BINDER_PORT " << ntohs(sin.sin_port) << std::endl;
}

void Binder::waitForActivity() {
  FD_ZERO(&readSet);
  FD_SET(mainSocket, &readSet);

  int maxFd = mainSocket;

  for (const auto& client : clients) {
    FD_SET(client.socket, &readSet);
    maxFd = std::max(maxFd, client.socket);
  }
  for (const auto& server : servers) {
    FD_SET(server.connection.socket, &readSet);
    maxFd = std::max(maxFd, server.connection.socket);
  }

  if (select(maxFd + 1, &readSet, nullptr, nullptr, nullptr) < 0) {
    fatalError("Error from select", -5);
  }
}

void Binder::checkForNewConnections() {
  if (!FD_ISSET(mainSocket, &readSet)) {
    return;
  }

  // Activity on the main socket means that there is a new client
  auto newSocket = accept(mainSocket, (struct sockaddr*)&sin, &sinLen);
  if (newSocket < 0) {
    // Error
    std::cerr << "Error accepting new socket" << std::endl;
  }
  clients.emplace_back(newSocket);
}

bool Binder::getMessage(Connection& connection, Message* message) {
  if (!FD_ISSET(connection.socket, &readSet)) {
    return false;
  }

  int finished = connection.read(&(message->type), &(message->message));
  if (finished < 0) {
    std::cerr << "Error on reading" << std::endl;
    message->type = Message::Type::INVALID;
    return true;
  }
  if (finished == 0) {
    // Not done reading yet
    return false;
  }

  return true;
}

void Binder::handleServerMessages() {
  Message message;
  auto i = servers.begin();
  while (i != servers.end()) {
    auto& server = *i;

    if (!getMessage(server.connection, &message)) {
      ++i;
      continue;
    }

    switch (message.type) {
    case Message::Type::REGISTRATION:
      // Function registration
      //auto signature = FunctionSignature::deserialize(receivedMessage);
      //auto server = ServerConnection(connection);
      //server.addSignature(signature);
      break;

    default:
      std::cerr << "Got unknown message " << message.message << std::endl;
      server.connection.close();
      i = servers.erase(i);
      break;
    }
  }
}

void Binder::handleClientMessages() {
  Message message;
  auto i = clients.begin();
  while (i != clients.end()) {
    auto& client = *i;
    if (!getMessage(client, &message)) {
      ++i;
      continue;
    }

    switch (message.type) {
    case Message::Type::TERMINATION:
      // We must terminate all servers
      for (auto& server : servers) {
        server.terminate();
      }
      servers.clear();
      stopped = true;

      client.close();
      // TODO: Stop the binder...
      break;

    case Message::Type::ADDRESS:
      // Client wanting server address
      //auto signature = FunctionSignature::deserialize(receivedMessage);
      //auto server = getServer(signature);
      //connection.send(messageType, server.serialize());
      client.close();
      break;

    case Message::Type::SERVER_REGISTRATION:
      // This client is actually a server registering itself
      // Do not close the connection
      servers.emplace_back(std::move(client));
      break;

    default:
      // Some invalid type
      std::cerr << "Saw invalid message " << message.message << std::endl;
      client.close();
      break;
    }
    i = clients.erase(i);
  }
}

void Binder::run() {
  while (!stopped) {
    waitForActivity();
    checkForNewConnections();
    handleClientMessages();
    handleServerMessages();
  }

  // If we still have any open, just terminate them now
  for (auto& client : clients) {
    client.close();
  }
  for (auto& server : servers) {
    // For some reason we still have some open
    server.terminate();
  }
  std::cerr << "System shutting down" << std::endl;
}
