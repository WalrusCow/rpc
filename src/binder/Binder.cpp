#include "binder/Binder.hpp"

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <sstream>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common/MessageType.hpp"

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

  for (const auto& connection : clients) {
    FD_SET(connection.socket, &readSet);
    maxFd = std::max(maxFd, connection.socket);
  }
  for (const auto& connection : servers) {
    FD_SET(connection.socket, &readSet);
    maxFd = std::max(maxFd, connection.socket);
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

void Binder::handleMessages(std::list<Connection>& connections,
                            const Binder::ConnectionCallback& callback) {
  auto i = connections.begin();
  while (i != connections.end()) {
    auto& connection = *i;
    if (!FD_ISSET(connection.socket, &readSet)) {
      i++;
      continue;
    }

    std::string receivedMessage;
    MessageType messageType;
    int finished = connection.read(&messageType, &receivedMessage);
    if (finished < 0) {
      std::cerr << "Error on reading" << std::endl;
      // Or we read nothing, so connection is closed anyway.
      connection.close();
      i = connections.erase(i);
      continue;
    }
    if (finished == 0) {
      // Not done reading yet
      i++;
      continue;
    }

    // Erase if the callback is true
    if (callback(connection, messageType, receivedMessage)) {
      i = connections.erase(i);
    }
  }
}

bool Binder::handleServerMessage(Connection& connection,
                                 MessageType messageType,
                                 const std::string& message) {
  switch (messageType) {
  case MessageType::REGISTRATION:
    // Function registration
    //auto signature = FunctionSignature::deserialize(receivedMessage);
    //auto server = ServerConnection(connection);
    //server.addSignature(signature);
    return false;

  default:
    std::cerr << "Got unknown message from server: " << message << std::endl;
    connection.close();
    return true;
  }
}

bool Binder::handleClientMessage(Connection& connection,
                                 MessageType messageType,
                                 const std::string& message) {
  switch (messageType) {
  case MessageType::TERMINATION:
    // We must terminate all servers
    //for (const auto& server : servers) {
    //  server.terminate();
    //}
    connection.close();
    return true;

  case MessageType::ADDRESS:
    // Client wanting server address
    //auto signature = FunctionSignature::deserialize(receivedMessage);
    //auto server = getServer(signature);
    //connection.send(messageType, server.serialize());
    connection.close();
    return true;

  case MessageType::SERVER_REGISTRATION:
    // This client is actually a server registering itself
    // Do not close the connection
    servers.push_back(std::move(connection));
    return true;

  default:
    // Some invalid type
    std::cerr << "Saw invalid message " << message << std::endl;
    return true;
  }
}

void Binder::run() {
  while(true) {
    waitForActivity();
    checkForNewConnections();
    handleMessages(
        servers, [=] (Connection& c, MessageType m, const std::string& s) {
      return handleServerMessage(c, m, s);
    });
    handleMessages(
        clients, [=] (Connection& c, MessageType m, const std::string& s) {
      return handleServerMessage(c, m, s);
    });
  }
}
