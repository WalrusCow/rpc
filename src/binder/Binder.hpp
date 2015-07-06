#pragma once

#include <list>
#include <string>
#include <sstream>

#include <netinet/in.h>

#include "common/Connection.hpp"
#include "binder/ServerConnection.hpp"

class Binder {
 public:
  void run();
  void connect();

 private:
  int mainSocket;
  struct sockaddr_in sin;
  socklen_t sinLen = sizeof(sin);

  // List of available servers
  std::list<ServerConnection> servers;
  std::list<Connection> connections;
  fd_set readSet;

  void waitForActivity();
  void checkForNewConnections();
  void handleConnections();
};
