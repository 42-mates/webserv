#pragma once
# include "cluster.hpp"				// Server: config: json
# include "CGIManager.hpp"
# include "ClientConnection.hpp"	// utils, Request
# include <fcntl.h>
# include <sys/wait.h>
# include <sstream>
# include <stdexcept>
# include "Request.hpp"

# define BUFFER_SIZE 30720
# define TIMEOUT_TIME 3

extern char**	environ;

class	ClientConnection;
class	CGIManager;
class	Cluster;

enum	reqType
{
	CHUNKED,
	NONCHUNKED
};

class	EventHandler
{
	public:
		EventHandler(const Cluster& cluster);
		~EventHandler();

		void		setNonBlock(int fd);
		bool		addToEpoll(int fd);
		bool		deleteFromEpoll(int fd);
		void		addClient(int clientFd, const Config& config);
		void		addServer(Server& s);
		void		handleNewConnection(Server& s);
		void		handleClientRequest(int clientFd);
		void		generateResponse(int clientFd);
		void		handleResponse(int clientFd);
		void		epollLoop(void);
		void		changeToWrite(int clientFd);
		void		changeToRead(int clientFd);
		size_t		getContentSize(int clientFd);
		bool		isBodyTooBig(int clientFd);
		bool		isResponseComplete(int clientFd);
		void		sendInvalidResponse(int clientFd);
		bool		isMultiPartReq(int clientFd);
		bool		isMultiPartReqFinished(int clientFd);
		CgiResult	startCGI(int clientFd, std::vector<std::string> fname);
		void		checkCompleteCGIProcesses(void);


		void			remove_client(int clientFd);
		const Config&	get_config(const std::string& host, int clientFd) const;
		void			handle_environment(const Request& req, const std::string& arg);

		std::string::size_type	getHeaderEndPos(int clientFd);
		bool					isHeaderChunked(int clientFd);
		bool					isChunkReqFinished(int clientFd);
		void					cleanChunkedReq(int clientFd);
		std::string				extractHeader(std::string & reqBuffer, std::string::size_type & headerEnd, int clientFd);
		std::string::size_type	getChunkSize(std::string::size_type& chunkStart,
												std::string::size_type& chunkEnd,
												std::string& reqBuffer);

		class	epollInitFailure;
		class	epollWaitFailure;

	private:
		enum CONNTYPE
		{
			EP_SERVER,
			EP_CLIENT,
			EP_CGI
		};

		int									_epollFd;
		std::map<int, Server*>				_servers;
		std::map<int, ClientConnection*>	_clients;
		CGIManager							_cgiManager;
		std::map<int, CONNTYPE>				_openConns;

		const Cluster&						_cluster;
};