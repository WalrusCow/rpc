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
  {
    Server* server = getServer(conn.socket);
    //TODO
    //if (server == nullptr) {
      // Shitttah!
    //}
    server->ready = true;
    return false;
  }

  case Message::Type::RPC_REGISTRATION:
  {
    // Function registration
    // Now see if we have this signature already in here?
    Server* server = getServer(conn.socket);
    //TODO
    //if (server == nullptr)
    server->signatures.emplace_back(FunctionSignature::deserialize(message));
    return false;
  }

  default:
    conn.close();
    return true;
  }
}

bool Binder::handleClientMessage(const Message& message, Connection& conn) {
  switch (message.type) {
  case Message::Type::TERMINATION:
    // We must terminate all servers
    for (auto& server : serverConnections.clients) {
      server.send(Message::Type::TERMINATION, "");
      server.close();
    }
    serverConnections.clients.clear();
    // All done
    socketServer.stop();

    conn.close();
    break;

  case Message::Type::ADDRESS:
  {
    // Client wanting server address
    auto signature = FunctionSignature::deserialize(message);
    Server* server = getServer(signature);
    if (server == nullptr) {
      // lol
      conn.close();
      break;
    }
    std::cerr << "Retrieved the server" << std::endl;
    conn.send(Message::Type::ADDRESS, server->address.serialize());
    conn.close();
    break;
  }

  case Message::Type::SERVER_REGISTRATION:
  {
    // This client is actually a server registering itself
    // Do not close the connection
    auto addr = ServerAddress::deserialize(message);
    std::cerr << "Registering server "<<addr.hostname<<":"<<addr.port<<std::endl;
    serverList.emplace_back(conn.socket, std::move(addr));

    serverConnections.clients.emplace_back(std::move(conn));
    break;
  }

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
    std::cerr << "Errored out of serve routine." << std::endl;
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
    for (auto& sig : server.signatures) {
      if (sig == signature) {
        // This is the one: move it to the back
        serverList.splice(serverList.end(), serverList, it);
        return &serverList.back();
      }
    }
    it++;
  }
  return nullptr;
}
