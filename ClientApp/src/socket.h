#ifndef __SIMPLETCPSOCKET__
#define __SIMPLETCPSOCKET__

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
typedef int SOCKET;
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

class SimpleTCPSocket {
 protected:
  SOCKET socket_;

  SimpleTCPSocket() {}
  virtual ~SimpleTCPSocket();
  void init();
  void cleanup();

 public:
  virtual int send(const char* buf, const int& size) const;
  virtual int recv(char* buf, const int& size) const;
  virtual bool close();
};

#endif  //__SIMPLETCPSOCKET__
