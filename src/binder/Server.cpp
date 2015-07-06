#include "binder/Server.hpp"

#include "common/Message.hpp"

void Server::terminate() {
  connection.send(Message::Type::TERMINATION, "");
  connection.close();
}
