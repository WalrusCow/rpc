#pragma once

#include <list>
#include <string>
#include <sstream>

#include <netinet/in.h>

#include "binder/Server.hpp"
#include "common/Connection.hpp"
#include "common/Message.hpp"

class Binder {
 public:
  void run();
  void connect();

 private:
  int mainSocket;
  struct sockaddr_in sin;
  socklen_t sinLen = sizeof(sin);

  bool stopped = false;

  // List of available servers
  std::list<Server> servers;
  std::list<Connection> clients;
  fd_set readSet;

  bool getMessage(Connection& connection, Message* message);

  void waitForActivity();
  void checkForNewConnections();
  void handleClientMessages();
  void handleServerMessages();
};
