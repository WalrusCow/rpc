#pragma once

#include <functional>
#include <list>
#include <string>

#include <netinet/in.h>

#include "common/Connection.hpp"
#include "common/Message.hpp"
#include "common/ServerAddress.hpp"

// Uses select to read from a bunch of sockets
class SocketServer {
 public:
  // Return true if we are *done* with the client (should delete)
  typedef std::function<bool(const Message&, Connection&)> MessageHandler;

  struct ClientList {
    ClientList(const std::string& name_, const MessageHandler& mh)
      : name(name_), messageHandler(mh) {}
    std::string name;
    std::list<Connection> clients;
    MessageHandler messageHandler;
  };

  bool connect(ServerAddress* addr);
  // Return true on clean stop; false if we stopped on error
  bool serve();

  void addClientList(ClientList* clientList);
  void stop();
  bool isStopped() {
    return stopped;
  }

 protected:

 private:
  int mainSocket;
  struct sockaddr_in sin;
  socklen_t sinLen = sizeof(sin);

  fd_set readSet;

  bool stopped = false;
  bool errored = false;

  void waitForActivity();
  void checkForNewConnections();
  void handleMessages();

  bool getMessage(Connection& connection, Message* message);

  std::list<ClientList*> clientLists;
};
