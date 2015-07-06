#pragma once

#include <list>
#include <string>
#include <sstream>

#include <netinet/in.h>

#include "common/Connection.hpp"

class Binder {
 public:
  void run();
  void connect();

 private:
  int mainSocket;
  struct sockaddr_in sin;
  socklen_t sinLen = sizeof(sin);

  // List of available servers
  std::list<Connection> servers;
  std::list<Connection> clients;
  fd_set readSet;

  void waitForActivity();
  void checkForNewConnections();

  // Return true if the connection is done with
  typedef std::function<bool(
      Connection&, MessageType, const std::string&)> ConnectionCallback;
  void handleMessages(std::list<Connection>& connections,
                      const ConnectionCallback& callback);

  bool handleClientMessage(Connection& connection,
                           MessageType messageType,
                           const std::string& message);

  bool handleServerMessage(Connection& connection,
                           MessageType messageType,
                           const std::string& message);
};
