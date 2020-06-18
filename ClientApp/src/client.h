#ifndef CLIENT_H
#define CLIENT_H

#include "socket.h"

#define MAXDATASIZE 100 // max number of bytes we can get at once 

class  SimpleTCPClientSocket : public SimpleTCPSocket {
 public:
  SimpleTCPClientSocket() {}
  virtual ~SimpleTCPClientSocket() {}

  virtual bool connect(const char* host);
};

#endif
