#pragma once

#include <string>

#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "rpc.h"

#include "common/Connection.hpp"
#include "common/SocketServer.hpp"

class Server {
 public:
  // Connect to the binder and open a socket for listening to clients
  bool connect();
  bool run();
  bool registerRpc(const std::string& name, int* argTypes, skeleton f);

 private:
  SocketServer server;
  ServerAddress serverAddress;

  struct hostent* binderServer;
  struct sockaddr_in binderAddr;
  int binderSocket;

  bool handleMessage(const Message& m, Connection& conn);
  bool handleBinderMessage(const Message& m, Connection& conn);

  SocketServer::MessageHandler messageHandler =
    [&] (const Message& m, Connection& c) {
      return handleMessage(m, c);
  };
  SocketServer::MessageHandler binderHandler =
    [&] (const Message& m, Connection& c) {
      return handleBinderMessage(m, c);
  };

  // TODO: Thread pool to service requests?
  //std::list<std::thread> pendingRequests;

  SocketServer::ClientList clients =
    SocketServer::ClientList("clients", messageHandler);
  SocketServer::ClientList binderClientList =
    SocketServer::ClientList("binder", binderHandler);

  bool connectToBinder();

  // Check if the binder has requested termination
  bool terminationRequested();
};
