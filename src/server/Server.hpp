#pragma once

#include <functional>
#include <list>
#include <string>

#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "rpc.h"

#include "common/Connection.hpp"
#include "common/FunctionCall.hpp"
#include "common/FunctionSignature.hpp"
#include "common/SocketServer.hpp"
#include "common/ThreadQueue.hpp"

class Server {
 public:
  // Connect to the binder and open a socket for listening to clients
  bool connect();
  bool run();
  int registerRpc(const std::string& name, int* argTypes, skeleton f);

 private:
  SocketServer server;
  ServerAddress serverAddress;

  const size_t numThreads = 5;

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

  // Functions that we support
  struct ServerFunction {
    ServerFunction(const FunctionSignature& sig, skeleton f)
        : signature(sig), function(f) {}
    FunctionSignature signature;
    skeleton function;
  };
  std::list<ServerFunction> registeredFunctions;

  ServerFunction* getFunction(const FunctionSignature& signature);

  SocketServer::ClientList clients =
    SocketServer::ClientList("clients", messageHandler);
  SocketServer::ClientList binderClientList =
    SocketServer::ClientList("binder", binderHandler);
  // Direct pointer to binder (ease of access0
  Connection* binderConnection;

  bool connectToBinder();

  // Check if the binder has requested termination
  bool terminationRequested();

  void handleCall(const Message& message, Connection& conn);

  struct Job {
    Job(Connection&& conn, FunctionCall&& fc)
        : connection(std::move(conn)), functionCall(std::move(fc)) {}
    Connection connection;
    FunctionCall functionCall;
  };
  ThreadQueue<Job> jobQueue;
  void threadWork();
  std::mutex accessMutex;
};
