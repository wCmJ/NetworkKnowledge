#include<WinSock2.h>

void Server()
{
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        //error
    }
    struct sockaddr_in *serv;
    serv = (sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    serv->sin_family = AF_INET;
    serv->sin_port = htons(1025); // convert u_short from host to TCP/IP network byte order(big endian)
    serv->sin_addr.s_addr = inet_addr("127.0.0.1");// convert a string containing dotted-demical address into a ULONG 
    
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int re = bind(sockfd, (sockaddr*)serv, sizeof(sockaddr));
    // if re == -1 error
    listen(sockfd, 4);
    // if listen return -1 error
    SOCKET newSocket = accept(sockfd, NULL, NULL);
    
    recv(newSocket, buffer, len, 0);// 0 means flags
    
    send(newSokcet, buf, len, 0);
    
}

void Client()
{
    WSADATA wsaData;
    sockaddr_in *p;
    //initialize p
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(sockfd, (sockaddr*)p, sizeof(sockaddr));
    send(sockfd, buf, len, 0);

    closesocket(sockfd);
    WSACleanup();
    

}


