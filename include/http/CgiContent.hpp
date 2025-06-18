#pragma once
# include "AHttpData.hpp"

class	CgiContent: public AHttpData
{
	public:
		CgiContent(const std::string& str);
		CgiContent(const CgiContent& other);
		CgiContent&	operator=(const CgiContent& other);
		virtual ~CgiContent();

		void	parseHead();
};