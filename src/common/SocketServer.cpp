#include "common/SocketServer.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

void SocketServer::addClientList(SocketServer::ClientList* clientList) {
  clientLists.push_back(clientList);
}

void SocketServer::waitForActivity() {
  FD_ZERO(&readSet);
  FD_SET(mainSocket, &readSet);

  int maxFd = mainSocket;

  for (const auto& clientList : clientLists) {
    for (const auto& client : clientList->clients) {
      FD_SET(client.socket, &readSet);
      maxFd = std::max(maxFd, client.socket);
    }
  }

  if (select(maxFd + 1, &readSet, nullptr, nullptr, nullptr) < 0) {
    // TODO: Error?
    return;
  }
}

bool SocketServer::connect(ServerAddress* addr) {
  // Connect to port
  mainSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (mainSocket == -1) {
    return false;
  }

  char hostnameBuffer[1024];
  gethostname(hostnameBuffer, sizeof(hostnameBuffer));
  hostnameBuffer[sizeof(hostnameBuffer) - 1] = '\0';
  addr->hostname = hostnameBuffer;

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = 0; // Next available

  if (bind(mainSocket, (struct sockaddr*) (&sin), sinLen) == -1) {
    return false;
  }

  // Listen with max 5 pending connections
  if (listen(mainSocket, 5) == -1) {
    return false;
  }

  // Get port number
  if (getsockname(mainSocket, (struct sockaddr*) (&sin), &sinLen) == -1) {
    return false;
  }

  addr->port = ntohs(sin.sin_port);
  return true;
}

void SocketServer::checkForNewConnections() {
  if (!FD_ISSET(mainSocket, &readSet)) {
    return;
  }

  // Activity on the main socket means that there is a new client
  auto newSocket = accept(mainSocket, (struct sockaddr*)&sin, &sinLen);
  if (newSocket < 0) {
    // Error
    std::cerr << "Error accepting new socket" << std::endl;
  }
  // Add to first list (default list) (?)
  clientLists.front()->clients.emplace_back(newSocket);
}

bool SocketServer::getMessage(Connection& connection, Message* message) {
  if (!FD_ISSET(connection.socket, &readSet)) {
    return false;
  }

  int finished = connection.read(message);
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

void SocketServer::handleMessages() {
  Message message;
  for (auto& clientList : clientLists) {
    auto i = clientList->clients.begin();
    while (i != clientList->clients.end()) {
      auto& client = *i;
      if (!getMessage(client, &message)) {
        // Nothing to read yet
        i++;
        continue;
      }
      // Pass to handler or whatever
      if (clientList->messageHandler(message, client))
        // Done with this client
        i = clientList->clients.erase(i);
      else
        i++;
    }
  }
}

bool SocketServer::serve() {
  while (!stopped && !errored) {
    waitForActivity();
    checkForNewConnections();
    handleMessages();
  }

  for (auto& clientList : clientLists) {
    for (auto& client : clientList->clients) {
      client.close();
    }
  }

  return !errored;
}

void SocketServer::stop() {
  stopped = true;
}
