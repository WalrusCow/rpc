#include "binder/Binder.hpp"

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <sstream>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "common/ServerAddress.hpp"

namespace {

void fatalError(const std::string& error, int exitCode) {
  std::cerr << error << std::endl;
  std::exit(exitCode);
}

} // Anonymous

void Binder::connect() {
  ServerAddress address;
  if (!socketServer.connect(&address)) {
    fatalError("Could not connect", -1);
  }
  std::cerr << "BINDER_ADDRESS " << address.hostname << std::endl;
  std::cerr << "BINDER_PORT " << address.port << std::endl;
  return;
}

bool Binder::handleServerMessage(const Message& message, Connection& conn) {
  switch (message.type) {
  case Message::Type::SERVER_READY:
    handleServerReady(message, conn);
    return false;

  case Message::Type::RPC_REGISTRATION:
    handleRpcRegistration(message, conn);
    return false;

  default:
    handleServerClose(message, conn);
    return true;
  }
}

bool Binder::handleClientMessage(const Message& message, Connection& conn) {
  switch (message.type) {
  case Message::Type::TERMINATION:
    handleTermination(message, conn);
    break;

  case Message::Type::ADDRESS:
    handleGetAddress(message, conn);
    break;

  case Message::Type::SERVER_REGISTRATION:
    handleServerRegistration(message, conn);
    break;

  default:
    // Some invalid type
    conn.close();
    break;
  }

  // True that we can remove this one
  return true;
}

void Binder::run() {
  socketServer.addClientList(&clientConnections);
  socketServer.addClientList(&serverConnections);

  if (!socketServer.serve()) {
    //std::cerr << "Errored out of serve routine." << std::endl;
  }
  return;
}

Server* Binder::getServer(int socket) {
  for (auto& server : serverList) {
    if (server.socket == socket) {
      return &server;
    }
  }
  return nullptr;
}

Server* Binder::getServer(const FunctionSignature& signature) {
  // Get next server to respond to this signature
  auto it = serverList.begin();
  while (it != serverList.end()) {
    auto& server = *it;
    if (server.ready) { // Only for servers that are ready to serve
      for (auto& sig : server.signatures) {
        if (sig == signature) {
          // This is the one: move it to the back
          Server newS(std::move(server));
          serverList.erase(it);
          serverList.emplace_back(std::move(newS));
          return &serverList.back();
        }
      }
    }
    it++;
  }
  return nullptr;
}

void Binder::handleServerReady(const Message& message, Connection& conn) {
  Server* server = getServer(conn.socket);
  if (server == nullptr) {
    // No matching server is found here
    conn.close();
    return;
  }
  conn.send(Message::Type::SERVER_READY, "");
  server->ready = true;
}

void Binder::handleRpcRegistration(const Message& message, Connection& conn) {
  // Function registration
  // Now see if we have this signature already in here?
  Server* server = getServer(conn.socket);
  if (server == nullptr) {
    // No matching server is found here
    conn.close();
    return;
  }
  server->signatures.emplace_back(FunctionSignature::deserialize(message));
  conn.send(Message::Type::RPC_REGISTRATION, "");
}

void Binder::handleTermination(const Message& message, Connection& conn) {
  // We must terminate all servers
  for (auto& server : serverConnections.clients) {
    server.send(Message::Type::TERMINATION, "");
    server.close();
  }
  serverConnections.clients.clear();
  // All done
  socketServer.stop();

  conn.close();
}

void Binder::handleGetAddress(const Message& message, Connection& conn) {
  // Client wanting server address
  auto signature = FunctionSignature::deserialize(message);
  Server* server = getServer(signature);
  if (server == nullptr) {
    // lol, close it on the client
    conn.close();
    return;
  }
  conn.send(Message::Type::ADDRESS, server->address.serialize());
  conn.close();
}

void Binder::handleServerRegistration(
    const Message& message, Connection& conn) {
  // This client is actually a server registering itself
  // Do not close the connection
  auto addr = ServerAddress::deserialize(message);
  serverList.emplace_back(conn.socket, std::move(addr));

  serverConnections.clients.emplace_back(std::move(conn));
  conn.send(Message::Type::SERVER_REGISTRATION, "");
}

void Binder::handleServerClose(const Message& message, Connection& conn) {
  // Close the connection and remove from our bookkeeping
  conn.close();
  auto it = serverList.begin();
  while (it != serverList.end()) {
    if (it->socket == conn.socket) {
      serverList.erase(it);
      return;
    }
    it++;
  }
}
