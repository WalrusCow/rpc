#pragma once

#include <list>
#include <string>

#include <netinet/in.h>

#include "binder/Server.hpp"
#include "common/Connection.hpp"
#include "common/Message.hpp"
#include "common/SocketServer.hpp"

class Binder {
 public:
  void run();
  void connect();

 private:
  SocketServer server;

  bool handleClientMessage(const Message& message, Connection& conn);
  bool handleServerMessage(const Message& message, Connection& conn);
  // List of available servers
  //std::list<Server> servers;

  SocketServer::MessageHandler clientHandler =
    [&] (const Message& m, Connection& c) {
      return handleClientMessage(m, c);
  };
  SocketServer::MessageHandler serverHandler =
    [&] (const Message& m, Connection& c) {
      return handleServerMessage(m, c);
  };
  SocketServer::ClientList clients = SocketServer::ClientList(
      "clients", clientHandler);
  SocketServer::ClientList servers = SocketServer::ClientList(
      "servers", serverHandler);
};
