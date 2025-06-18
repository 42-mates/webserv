#pragma once
# include "utils.hpp"
# include "EventHandler.hpp"
# include "Request.hpp"

class	ClientConnection
{
	public:
		ClientConnection(int fd, const Config& config);
		ClientConnection(const ClientConnection& other);
		ClientConnection&	operator=(const ClientConnection& other);
		~ClientConnection();

		enum	reqType
		{
			CHUNKED,
			NONCHUNKED
		};

		int				_clientFd;
		std::string		_requestBuffer;
		std::string		_responseBuffer;
		std::string		_cgiBuffer;
		size_t			_bytesSent;
		bool			_responseReady;
		CgiResult		_cgiResult;
		bool			_cgi;
		reqType			_reqType;

		int				_errorCode;

		const Config&	_config;
		Request*		requestHTTP;

		void			resetData();
		void			closeConnection();
};