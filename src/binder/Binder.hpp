#pragma once

#include <list>
#include <map>
#include <string>

#include <netinet/in.h>

#include "binder/Server.hpp"
#include "common/Connection.hpp"
#include "common/Message.hpp"
#include "common/ServerAddress.hpp"
#include "common/SocketServer.hpp"

class Binder {
 public:
  void run();
  void connect();

 private:
  SocketServer socketServer;

  SocketServer::MessageHandler clientHandler =
    [&] (const Message& m, Connection& c) {
      return handleClientMessage(m, c);
  };
  SocketServer::MessageHandler serverHandler =
    [&] (const Message& m, Connection& c) {
      return handleServerMessage(m, c);
  };
  SocketServer::ClientList clientConnections = SocketServer::ClientList(
      "clients", clientHandler);
  SocketServer::ClientList serverConnections = SocketServer::ClientList(
      "servers", serverHandler);

  // A list of available servers
  std::list<Server> serverList;

  bool handleClientMessage(const Message& message, Connection& conn);
  bool handleServerMessage(const Message& message, Connection& conn);

  Server* getServer(int socket);
  Server* getServer(const FunctionSignature& signature);

  void handleServerReady(const Message& message, Connection& conn);
  void handleRpcRegistration(const Message& message, Connection& conn);
  void handleTermination(const Message& message, Connection& conn);
  void handleGetAddress(const Message& message, Connection& conn);
  void handleServerRegistration(const Message& message, Connection& conn);
};
