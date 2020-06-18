#include "socket.h"
#include <unistd.h>

#define INVALID_SOCKET -1

SimpleTCPSocket::~SimpleTCPSocket(){
    close();
}

void SimpleTCPSocket::init(){
    #ifdef _WIN32
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    #endif
}

void SimpleTCPSocket::cleanup(){
    #ifdef _WIN32
    WSACleanup();
    #endif
}

int SimpleTCPSocket::send(const char* buf, const int& size) const{
    if (socket_ == INVALID_SOCKET)
        return SOCKET_ERROR;
    return ::send(socket_, buf, size, 0);
}

int SimpleTCPSocket::recv(char* buf, const int& size) const{
    if (socket_ == INVALID_SOCKET)
        return SOCKET_ERROR;
    return ::recv(socket_, buf, size, 0);
}

bool SimpleTCPSocket::close(){
    int result = ::close(socket_);
    socket_ = INVALID_SOCKET;
    if(result == SOCKET_ERROR)
        return false;
    return true;
}
