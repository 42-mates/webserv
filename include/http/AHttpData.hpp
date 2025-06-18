#pragma once
# include <iostream>
# include <algorithm>
# include <fstream>
# include <string>
# include <map>
# include <vector>

class	AHttpData
{
	public:
		AHttpData(const std::string& str, bool isRequest);
		AHttpData(const AHttpData& other);
		AHttpData&	operator=(const AHttpData& other);
		virtual ~AHttpData();

		/* getters */
		const std::string&								getHeaderValue(const std::string& key) const;
		const std::multimap<std::string, std::string>&	getHeaders() const;
		const std::string&								getBody() const;
		const std::string&								getRaw() const;
		/* parser */
		virtual void	parseHead() = 0;
		bool			parseHeaders();
		/* utils */
		static std::string	trim(const std::string& str);

	protected:
		std::multimap<std::string, std::string>	_headers;
		std::string								_body;
		bool									_hasHeaders;
		const std::string						_raw;
		const bool								_isRequest;
};