#include "binder/Binder.hpp"

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <sstream>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common/ServerAddress.hpp"

namespace {

void fatalError(const std::string& error, int exitCode) {
  std::cerr << error << std::endl;
  std::exit(exitCode);
}

} // Anonymous

void Binder::connect() {
  ServerAddress address;
  if (!server.connect(&address)) {
    fatalError("Could not connect", -1);
  }
  std::cerr << "BINDER_ADDRESS " << address.hostname << std::endl;
  std::cerr << "BINDER_PORT " << address.port << std::endl;
  return;
}

bool Binder::handleServerMessage(const Message& message, Connection& conn) {
  switch (message.type) {
  case Message::Type::SERVER_READY:
    return false;
  case Message::Type::RPC_REGISTRATION:
  {
    // Function registration
    auto signature = FunctionSignature::deserialize(message);
    if (serverSignatures.find(conn.socket) == serverSignatures.end()) {
      serverSignatures[conn.socket] = {};
    }
    serverSignatures[conn.socket].push_back(signature);
    return false;
  }

  default:
    conn.close();
    return true;
  }
}

bool Binder::handleClientMessage(const Message& message, Connection& client) {
  switch (message.type) {
  case Message::Type::TERMINATION:
    // We must terminate all servers
    for (auto& conn : servers.clients) {
      conn.send(Message::Type::TERMINATION, "");
      conn.close();
    }
    servers.clients.clear();
    // All done
    server.stop();

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
    servers.clients.emplace_back(std::move(client));
    break;

  default:
    // Some invalid type
    client.close();
    break;
  }

  // True that we can remove this one
  return true;
}

void Binder::run() {
  server.addClientList(&clients);
  server.addClientList(&servers);

  if (!server.serve()) {
    std::cerr << "Errored out of serve routine." << std::endl;
  }
  return;
}
