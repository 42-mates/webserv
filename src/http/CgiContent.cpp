#include "CgiContent.hpp"

CgiContent::CgiContent(const std::string& str): AHttpData(str, false) {}

CgiContent::CgiContent(const CgiContent& other): AHttpData(other.getRaw(), false)
{*this = other;}

CgiContent&	CgiContent::operator=(const CgiContent& other)
{
	if (this != &other)
		AHttpData::operator=(other);
	return *this;
}

CgiContent::~CgiContent() {}

void	CgiContent::parseHead() {}