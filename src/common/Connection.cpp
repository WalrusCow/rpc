#include "Connection.hpp"

#include <algorithm>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

const uint32_t Connection::BUFFER_LEN = 1024;

Connection::Connection(int socket_) : socket(socket_) {
  struct sockaddr_in peerAddr;
  socklen_t sz = sizeof(peerAddr);
  if (getpeername(socket, (struct sockaddr*) &peerAddr, &sz) < 0) {
    // Failure...
    return;
  }

  hostname = inet_ntoa(peerAddr.sin_addr);
  port = ntohs(peerAddr.sin_port);
}

void Connection::close() {
  ::close(socket);
}

int Connection::send(Message::Type type, const std::string& str) {
  // NOTE: Send length does not include type size
  uint32_t length = str.size();
  char buffer[BUFFER_LEN];

  // Add length and type to the message
  std::memcpy(buffer, &length, sizeof(length));
  std::memcpy(buffer + sizeof(length), &type, sizeof(type));

  const uint32_t writeSz = sizeof(length) + sizeof(type);
  auto bytesWritten = write(socket, buffer, writeSz);
  if (bytesWritten < writeSz) {
    // Could not even write length and type: We'll call it an error
    return -1;
  }

  const char* cStr = str.c_str();
  uint32_t toWrite = length;
  // While we have not yet written everything...
  while (toWrite > 0) {
    auto toCopy = std::min(BUFFER_LEN, toWrite);
    std::memcpy(buffer, cStr, toCopy);
    bytesWritten = write(socket, buffer, toCopy);
    if (bytesWritten <= 0) {
      // Error or unable to write
      return -1;
    }
    // We have written something
    cStr += bytesWritten;
    toWrite -= bytesWritten;
  }

  // Wrote everything: success
  return 0;
}

int Connection::doRead(
    Message* message,
    const std::function<ssize_t(int, char*, size_t)>& reader) {
  char buffer[BUFFER_LEN];

  ssize_t bytesReceived = 0;
  if (bytesToRead == 0) {
    ss.str("");
    ss.clear();
    const uint32_t readSz = sizeof(bytesToRead) + sizeof(messageType);
    // We have to read the messagelength
    bytesReceived = reader(socket, buffer, readSz);
    if (bytesReceived == 0) {
      // Closed
      return -2;
    }
    if (bytesReceived < readSz) {
      // Error or couldn't read enough
      return -1;
    }
    const auto sz = sizeof(bytesToRead); // shorthand
    std::memcpy(&bytesToRead, buffer, sz);
    std::memcpy(&messageType, buffer + sz, sizeof(messageType));
  }

  if (bytesToRead == 0) {
    // If we still have nothing to read then there *is* nothing to read.
    message->message = "";
    message->type = messageType;
    return 1;
  }

  do {
    // Read as much as we can
    uint32_t bytesToLoad = std::min(BUFFER_LEN, bytesToRead);
    bytesReceived = reader(socket, buffer, bytesToLoad);
    if (bytesReceived < 0) {
      // Error in reading
      return -1;
    }

    bytesToRead -= bytesReceived;
    // Add to string buffer
    ss.write(buffer, bytesReceived);
  } while (bytesReceived > 0 && bytesToRead > 0);

  // We have read the whole message
  if (bytesToRead <= 0) {
    message->message = ss.str();
    message->type = messageType;
    // Done
    return 1;
  }
  // Not done
  return 0;
}

int Connection::read(Message* message) {
  return doRead(message, [&] (int sfd, char* buffer, size_t toRead) {
    return ::read(sfd, buffer, toRead);
  });
}
