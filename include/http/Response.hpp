#pragma once
# include "Request.hpp"			// AHttpData
# include "EventHandler.hpp"	// cluster: Server: config: json; CGIManager: ClientConnection: utils, Request
# include <ctime>

class	Response
{
	public:
		enum	TimeForm
		{
			STANDARD,
			SIMPLE,
		};

		Response(const Config& config, int errCode);
		Response(const Request& request, const Config& config);
		Response(const Request& request, const Config& config, const std::string& cgi_content, CgiResult cgi_res);
		~Response();

		std::string		generateResponse();
		/* static data set init */
		static const std::map<int, std::string>						initStatusCodes();
		static const std::vector<std::string>						init_allowed_cgi();
		static const std::map<std::string, std::string>				initAcceptedFile();
		static const std::map<std::string, std::string>				initAcceptedFileReversed();
		static bool													check_extension(const std::string& url);
		static Config::Route const&									find_match(const Config& config, const std::string& url);
		static std::map<std::string, std::string>::const_iterator	check_cgi(const Config::Route& route, const std::string& url);
		static std::string											check_postFile(const std::string& type);
		std::string													getErrorContent(int errCode);

	private:
		/* data */
		std::string								_content;
		int										_statusCode;
		std::string								_contentType;
		Config::Route							_routes;
		std::multimap<std::string, std::string>	_extraHeaders;
		Config const&							_config;

		/* init utils */
		std::string								readFile(const std::string& filename);
		std::string								getFileContent(const std::string& url);
		std::string								getPostContent(const Request& request);
		std::string								getDeleteContent(const Request& request);

		/* static functions */
		static std::string						toLower(std::string s);
		static std::string						getCurrentTime(Response::TimeForm mode);

		/* class level const data set */
		static const std::map<int, std::string>			_statusCodes;
		static const std::vector<std::string>			_allowedCGI;
		static const std::map<std::string, std::string>	_acceptedFile;
		static const std::map<std::string, std::string>	_acceptedFileReversed;

		/* DIRECTORY LISTING */
		static const std::string	_html_template;

		std::string					directory_listing(const std::string& path, const std::string& file);
};