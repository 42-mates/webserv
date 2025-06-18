#include "AHttpData.hpp"

AHttpData::AHttpData(const std::string& str, bool isRequest): _raw(str), _isRequest(isRequest)
{
	_hasHeaders = parseHeaders();
	if (!_hasHeaders)
		_body = _raw;
}

AHttpData::AHttpData(const AHttpData& other): _raw(other._raw), _isRequest(other._isRequest)
{*this = other;}

AHttpData&	AHttpData::operator=(const AHttpData& other)
{
	if (this != &other)
	{
		this->_body = other._body;
		this->_headers = other._headers;
	}
	return *this;
}

AHttpData::~AHttpData() {}

const std::string&	AHttpData::getHeaderValue(const std::string& key) const
{
	std::multimap<std::string, std::string>::const_iterator	it = this->_headers.find(key);

	if (it != this->_headers.end())
		return it->second;
	throw std::runtime_error("Key not found in _headers.\n");
}

const std::multimap<std::string, std::string>&	AHttpData::getHeaders() const
{return this->_headers;}

const std::string&	AHttpData::getBody() const
{return this->_body;}

const std::string&	AHttpData::getRaw() const
{return this->_raw;}

// Parse headers (key, value)
bool	AHttpData::parseHeaders()
{
	std::string::const_iterator	it = _raw.begin();
	std::string::const_iterator	end = _raw.end();

	// If the concrete class is a Request, offset to the 2nd line
	// 	跳過第一行（請求行，例如 GET /index.html HTTP/1.1），從第二行開始處理，
	// 	因為第二行才是頭字段的開始（例如 Host: kyeh.42.fr）
	if (_isRequest)
	{
		it = std::find(it, end, '\n');	// Move to the end of first line
		if (it == end)
			return false;
		++it;
	}

	// Check if the header ends correctly (\r\n\r\n: HTTP 標準的頭字段結束標記，參考 RFC 7230)
	std::string::size_type	headerEnd = _raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
	{
		headerEnd = _raw.find("\n\n");	//兼容非標準的換行格式，增加容錯性。
		if (headerEnd == std::string::npos)
			return false;
	}

	// Verify the header's structure
	std::string				headerSection = _raw.substr(std::distance(_raw.begin(), it), headerEnd);
	std::string::size_type	firstColon = headerSection.find(':');
	std::string::size_type	firstNewline = headerSection.find('\n');

	if (firstColon == std::string::npos || (firstNewline != std::string::npos && firstColon > firstNewline))
		return false;
	while (it != end)
	{
		// End of line
		std::string::const_iterator	line_end = std::find(it, end, '\n');
		// Empty line or line of whitespaces
		if (line_end == it || (line_end != end && *(line_end - 1) == '\r' && line_end - 1 == it))
			break ;
		std::string::const_iterator	colon = std::find(it, line_end, ':');
		if (colon == line_end)	// Invalid line: no colon. Skip and continue
		{
			it = line_end + 1;
			continue ;
		}

		std::string	key(it, colon);
		std::string	value(colon + 1, line_end);
		key = trim(key);
		value = trim(value);
		if (!value.empty() && value[value.length() - 1] == '\r')
			value.erase(value.length() - 1);
		if (!key.empty())
			_headers.insert(std::pair<std::string, std::string>(key,value));
		// std::cout << "Key: [" << key << "] | Value: [" << value << "]" << std::endl;
		it = line_end + 1;
	}

	// Parese the body if there's any
	if (it != end && std::find(it, end, '\n') != end)
		_body.assign(std::find(it, end, '\n') + 1, end);
	return true;
}

std::string	AHttpData::trim(const std::string& str)
{
	std::string::const_iterator	start = std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(isspace)));
	std::string::const_iterator	end = std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base();
	return start < end ? std::string(start, end) : std::string();
}