#include "client.h"
#include <iostream>

#define PORT_NUM 3490

bool SimpleTCPClientSocket::connect(const char* host){
    struct sockaddr_in myaddr;
    struct hostent *hostName = gethostbyname(host);

    if(hostName == NULL)
        return false;

    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(PORT_NUM);
    myaddr.sin_addr = *((struct in_addr *)hostName->h_addr);

    socket_ = socket(PF_INET, SOCK_STREAM, 0);

    if(socket_ == SOCKET_ERROR)
        return false;
        
    int result = ::connect(socket_, (struct sockaddr*)&myaddr, sizeof myaddr);

    if(result == SOCKET_ERROR)
        return false;
    return true;
}