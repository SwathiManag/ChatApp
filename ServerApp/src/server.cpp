#include "server.h"
#include <iostream>
#include <unistd.h>

#define PORT_NUM 3490

SimpleTCPServerSocket::~SimpleTCPServerSocket(){
    closeserver();
}

bool SimpleTCPServerSocket::bind(){
    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(PORT_NUM);
    myaddr.sin_addr.s_addr = INADDR_ANY;

    serversocket_ = socket(PF_INET, SOCK_STREAM, 0);
    int result = ::bind(serversocket_, (struct sockaddr*)&myaddr, sizeof myaddr);

    if(result == SOCKET_ERROR)
      return false;
    
    return true;
}

bool SimpleTCPServerSocket::listen(){
    int result = ::listen(serversocket_, BACKLOG);
    if(result == SOCKET_ERROR)
        return false;
    return true;
}

bool SimpleTCPServerSocket::accept(){
    struct sockaddr_in myaddr;
    socklen_t addr_size;
    addr_size = sizeof myaddr;
    socket_ = ::accept(serversocket_, (struct sockaddr*)&myaddr, &addr_size);
    if(socket_ == SOCKET_ERROR)
        return false;
    return true;
}

bool SimpleTCPServerSocket::closeserver(){
    int result = ::close(serversocket_);
    if(result == SOCKET_ERROR)
        return false;
    return true;
}