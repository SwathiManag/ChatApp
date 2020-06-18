#ifndef SERVER_H
#define SERVER_H

#include "socket.h"

#define BACKLOG 10   // how many pending connections queue will hold

class  SimpleTCPServerSocket : public SimpleTCPSocket {
 protected:
  SOCKET serversocket_;

 public:
  SimpleTCPServerSocket() {}
  virtual ~SimpleTCPServerSocket();

  virtual bool bind();
  virtual bool listen();
  virtual bool accept();
  virtual bool closeserver();
};

#endif


