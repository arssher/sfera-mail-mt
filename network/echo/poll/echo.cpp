#include <iostream>
#include <algorithm>
#include <set>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <errno.h>
#include <string.h>

#define POLL_SIZE 32

int main(int argc, char **argv)
{
	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	std::set<int> SlaveSockets;

	if(MasterSocket == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(12345);
	SockAddr.sin_addr.s_addr = INADDR_ANY;

	int Result = bind(MasterSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr));

	if(Result == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	Result = listen(MasterSocket, SOMAXCONN);

	if(Result == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	struct pollfd Set[POLL_SIZE];
	Set[0].fd = MasterSocket;
	Set[0].events = POLLIN;

	while(true)
	{
		unsigned int Index = 1;
		for(auto Iter = SlaveSockets.begin(); Iter != SlaveSockets.end(); Iter++)
		{
			Set[Index].fd = *Iter;
			Set[Index].events = POLLIN;
			Index++;
		}
		unsigned int SetSize = 1 + SlaveSockets.size();

		poll(Set, SetSize, -1);
		
		for(unsigned int i = 0; i < SetSize; i++)
		{
			if(Set[i].revents & POLLIN)
			{
				if(Set[i].fd == MasterSocket)
				{
					int SlaveSocket = accept(MasterSocket, 0, 0);
					SlaveSockets.insert(SlaveSocket);
				}
				else
				{
					static char Buffer[1024];
					int RecvSize = recv(Set[i].fd, Buffer, 1024, MSG_NOSIGNAL);
					if((RecvSize == 0) && (errno != EAGAIN))
					{
						shutdown(Set[i].fd, SHUT_RDWR);
						close(Set[i].fd);
						SlaveSockets.erase(Set[i].fd);
					}
					else if(RecvSize != 0)
					{
						send(Set[i].fd, Buffer, RecvSize, MSG_NOSIGNAL);
					}

				}
			}
		}
	}

	return 0;
}
