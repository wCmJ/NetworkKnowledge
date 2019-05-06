#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<poll.h>
#include<string.h>
#include<iostream>
#include<vector>
#include<errno.h>

using namespace std;

int main()
{
	int listenfd = socket(AF_INET,SOCK_STREAM, 0);
	if(listenfd == -1)
	{
		cout<<"socket error"<<endl;
		return -1;
	}

	int on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	//setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on));

	int oldSocketFlag = fcntl(listenfd, F_GETFL, 0);
	int newSocketFlag = oldSocketFlag | O_NONBLOCK;
	if(fcntl(listenfd, F_SETFL, newSocketFlag) == -1)
	{
		close(listenfd);
		cout<<"set listenfd to nonblock error"<<endl;
		return -1;
	}

	struct sockaddr_in bindaddr;
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_port = htons(50000);
	bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1)
	{
		close(listenfd);
		cout<<"bind error"<<endl;
		return -1;
	}

	if(listen(listenfd, SOMAXCONN) == -1)
	{
		close(listenfd);
		cout<<"listen error"<<endl;
		return -1;
	}

	int epollfd = epoll_create(1);
	if(epollfd == -1)
	{
		cout<<"create epoll error"<<endl;
		close(listenfd);
		return -1;
	}

	epoll_event listen_fd_event;
	listen_fd_event.data.fd = listenfd;
	listen_fd_event.events = EPOLLIN;// use LT
	//listen_fd_event.events != EPOLLET;// use ET

	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &listen_fd_event) == -1)
	{
		cout<<"epoll ctl error"<<endl;
		close(listenfd);
		return -1;
	}


	int n;
	while(true)
	{
		epoll_event epoll_events[1024];
		n = epoll_wait(epollfd, epoll_events, 1024, 1000);
		if(n < 0)
		{
			if(errno == EINTR)
				continue;
			//error
			break;
		}
		else if(n == 0)
		{
			//timeout
			continue;
		}
		else
		{
			for(size_t i =0;i<n;++i)
			{
				if(epoll_events[i].events & EPOLLIN)
				{
					if(epoll_events[i].data.fd == listenfd)
					{
						//accept new connect
						struct sockaddr_in clientaddr;
						socklen_t clientaddrlen = sizeof(clientaddr);
						int clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientaddrlen);
						if(clientfd != -1)
						{
							int oldSocketFlag = fcntl(clientfd, F_GETFL, 0);
							int newSocketFlag = oldSocketFlag | O_NONBLOCK;
							if(fcntl(clientfd, F_SETFL, newSocketFlag) == -1)
							{
								cout<<"set clientfd to nonblock error"<<endl;
								close(clientfd);
							}
							else
							{
								epoll_event client_fd_event;
								client_fd_event.data.fd = clientfd;
								client_fd_event.events = EPOLLIN;// use LT
								//client_fd_event.events |= EPOLLET// use ET
								if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &client_fd_event) != -1)
								{
									cout<<"new client accepted, clientfd: "<<clientfd<<endl;
								}
								else
								{
									cout<<"add clientfd to epollfd error"<<endl;
									close(clientfd);
								}

							}

						}
					}
					else
					{
						cout<<"clientfd: "<<epoll_events[i].data.fd<<"recv data"<<endl;
						char ch;//receive one byte
						int m = recv(epoll_events[i].data.fd, &ch, 1, 0);
						if(m < 0)
						{
							if(errno != EWOULDBLOCK && errno != EINTR)
							{
								if(epoll_ctl(epollfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, NULL) != -1)
								{
									cout<<"client disconnect, client fd: "<<epoll_events[i].data.fd<<endl;
								}
								close(epoll_events[i].data.fd);
							}
						}
						if(m == 0)
						{
							//peer close connect
							if(epoll_ctl(epollfd, EPOLL_CTL_DEL, epoll_events[i].data.fd, NULL) != -1)
							{
								cout<<"client disconnect, client fd: "<<epoll_events[i].data.fd<<endl;
							}
							close(epoll_events[i].data.fd);
						}
						else
						{
							cout<<"recv normal data, client fd: "<<epoll_events[i].data.fd << " data is: " <<ch<<endl;
						}

					}
				}
				else if(epoll_events[i].events & POLLERR)
				{
					//todo
				}
			}

		}


	}
	close(listenfd);
	return 0;
}
