#include "SocketManager.hpp"
#include "Configuration.hpp"

#include <poll.h>

#include <vector>

namespace pollfds
{
	int add_server_fd(std::vector<struct pollfd> &pfds, std::vector<struct ServerSocket> &servers)
	{
		if (servers.empty())
		{
			return 0;
		}
		std::vector<struct ServerSocket>::iterator it;
		for (it = servers.begin(); it != servers.end(); it++)
		{
			struct pollfd pfd;
			pfd.fd = it->socket;
			pfd.events = POLLIN;
			pfds.push_back(pfd);
		}
		return (servers.size());
	}
}

int main(int argc, char **argv)
{
	int server_running = 1;
	Configuration conf;
	SocketManager sm;
	std::vector<struct pollfd> pfds;

	sm.set_servers(conf.getPorts());

	//add servers to pollfd as listening port and set events to POLLIN
	int server_fd_count = pollfds::add_server_fd(pfds, sm.get_servers());
	if (server_fd_count == 0)
	{
		std::cerr << "No servers to listen on" << std::endl;
		exit (-1);
	}
	int fd_count = server_fd_count;
	while (server_running)
	{
		//poll for events
		int poll_count = poll(pfds.data(), fd_count, -1);
		if (poll_count == -1)
		{
			std::cerr << "poll: " << strerror(errno) << std::endl;
			exit (-1);
		}
		//check for events
		for (int i = 0; i < fd_count; i++)
		{
			if (pfds[i].revents & POLLIN)
			{
				if (pfds[i].fd == sm.get_servers()[i].socket)
				{
					//accept connection
					//add client to pollfd
				}
				else
				{
					//recv from client
					//process request
				}
			}
			else if (pfds[i].revents & POLLOUT)
			{
				//send response to client
			}
		}
	}
}
