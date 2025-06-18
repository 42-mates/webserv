#pragma once
# include "config.hpp"
# include <netinet/in.h>
# include <sys/epoll.h>
# include <unistd.h>
# include <asm-generic/socket.h>

# define MAX_EVENTS 1024
# define MAX_CLIENTS 500

class	Server
{
	public:
		Server(Config& config);
		Server(const Server& socket);
		Server&	operator=(const Server& other);
		~Server();

		int				getSockFd() const;
		int				getPort() const;
		int				getMaxClients() const;
		int				getMaxEvents() const;
		const Config&	getConfig() const;

		class	socketCreationFailure;
		class	socketBindFailure;
		class	socketListenFailure;

	private:
		Config&	_config;

		int		_sockFd;
		int		_port;
		int		_maxClients;
		int		_maxEvents;
};