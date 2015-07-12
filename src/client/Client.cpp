#include "client/Client.hpp"

#include <cstring>
#include <iostream>

#include "common/FunctionCall.hpp"
#include "common/Message.hpp"
#include "common/ServerAddress.hpp"

int Client::connectTo(const std::string& host, int port) const {
  struct hostent* server;
  struct sockaddr_in addr;

  server = gethostbyname(host.c_str());
  if (server == nullptr) {
    return -1;
  }
  addr.sin_family = AF_INET;
  std::memcpy((char*)&addr.sin_addr.s_addr,
              (char*)server->h_addr,
              (int)server->h_length);
  addr.sin_port = htons(port);

  int sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd < 0) {
    return -1;
  }

  if (connect(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    return -1;
  }
  return sfd;
}

Client::Client() {
  char* binderHostname = getenv("BINDER_ADDRESS");
  if (!binderHostname) {
    std::cerr << "Could not get BINDER_ADDRESS from environment!" << std::endl;
    return;
  }
  char* portStr = getenv("BINDER_PORT");
  if (!portStr) {
    std::cerr << "Could not get BINDER_PORT from environment!" << std::endl;
    return;
  }
  binderHost = binderHostname;
  // Binder information
  binderPort = std::stoi(portStr);
}

int Client::connectToServer(const FunctionSignature& signature) const {
  int binderSocket = connectTo(binderHost, binderPort);
  if (binderSocket < 0) {
    return binderSocket;
  }
  Connection binderConnection(binderSocket);

  // Send a request for the server address
  binderConnection.send(Message::Type::ADDRESS, signature.serialize());
  // Read the response for the server address
  Message message;
  binderConnection.read(&message);
  binderConnection.close();
  // Okay, now we have the message with the server address
  auto serverAddress = ServerAddress::deserialize(message);
  std::cerr << "Got server address " << serverAddress.hostname
            << ":" << serverAddress.port << std::endl;

  return connectTo(serverAddress.hostname, serverAddress.port);
}

int Client::rpcCall(const std::string& fun, int* argTypes, void** args) const {
  FunctionSignature signature(fun, argTypes);
  int serverSocket = connectToServer(signature);
  if (serverSocket < 0) {
    return serverSocket;
  }

  Connection conn(serverSocket);

  FunctionCall functionCall(std::move(signature), args);
  if (conn.send(Message::Type::CALL, functionCall.serialize()) < 0) {
    conn.close();
    return -1;
  }

  Message message;
  while (true) {
    int r = conn.read(&message);
    if (r < 0) {
      conn.close();
      return -1;
    }
    else if (r > 0) {
      // Done
      conn.close();
      break;
    }
  }

  // Now we must deserialize the message
  auto callReturn = FunctionCall::deserialize(message);
  // Got the response back
  std::cerr << "Got call response. Signatures equal? "
            << (callReturn.signature == functionCall.signature) << std::endl;
  callReturn.writeDataTo(args);
  return 0;
}

int Client::terminate() const {
  int sfd = connectTo(binderHost, binderPort);
  if (sfd < 0) {
    return sfd;
  }
  Connection conn(sfd);
  return conn.send(Message::Type::TERMINATION, "");
}
