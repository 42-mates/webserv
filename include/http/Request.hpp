#pragma once
# include "AHttpData.hpp"

class	Request: public AHttpData
{
	public:
		Request(const std::string& str);
		Request(const Request& other);
		Request&	operator=(const Request &other);
		virtual ~Request();

		/* getters */
		const std::string&	getMethod() const;
		const std::string&	getUrl() const;
		const std::string&	getHttpVersion() const;
		const std::string&	getQueryString() const;

		/* Utils */
		void				parseHead();
		void				printAll();

	private:
		std::string	_method;
		std::string	_url;
		std::string	_http_version;
		std::string	_query_string;
};