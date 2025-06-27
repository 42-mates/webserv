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

// Parse headers (key: value pairs) from HTTP data
bool	AHttpData::parseHeaders()
{
	std::string::const_iterator	it = _raw.begin();
	std::string::const_iterator	end = _raw.end();

	// If parsing a Request, skip the first line (request line)
	// Example: "GET /index.html HTTP/1.1" - this is not a header
	// Headers start from the second line (e.g., "Host: localhost")
	if (_isRequest)
	{
		it = std::find(it, end, '\n');	// Move to end of first line
		if (it == end)
			return false;
		++it;  // Start from second line
	}

	// Check if headers end correctly with \r\n\r\n (HTTP standard, RFC 7230)
	std::string::size_type	headerEnd = _raw.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
	{
		// Also check for \n\n for compatibility with non-standard formats
		headerEnd = _raw.find("\n\n");
		if (headerEnd == std::string::npos)
			return false;  // No header end marker found
	}

	// Verify header structure (must have at least one valid header)
	std::string				headerSection = _raw.substr(std::distance(_raw.begin(), it), headerEnd);
	std::string::size_type	firstColon = headerSection.find(':');
	std::string::size_type	firstNewline = headerSection.find('\n');

	if (firstColon == std::string::npos || (firstNewline != std::string::npos && firstColon > firstNewline))
		return false;
	
	// Parse each header line
	while (it != end)
	{
		// Find end of current line
		std::string::const_iterator	line_end = std::find(it, end, '\n');
		// Check for empty line (end of headers)
		if (line_end == it || (line_end != end && *(line_end - 1) == '\r' && line_end - 1 == it))
			break;  // Empty line marks end of headers
		
		// Find colon separator between key and value
		std::string::const_iterator	colon = std::find(it, line_end, ':');
		if (colon == line_end)	// Invalid header: no colon found
		{
			it = line_end + 1;
			continue;  // Skip invalid line
		}

		// Extract key and value
		std::string	key(it, colon);           // e.g., "Content-Type"
		std::string	value(colon + 1, line_end); // e.g., " text/html"
		// Remove whitespace
		key = trim(key);
		value = trim(value);
		// Remove trailing \r if present
		if (!value.empty() && value[value.length() - 1] == '\r')
			value.erase(value.length() - 1);
		// Store header if key is not empty
		if (!key.empty())
			_headers.insert(std::pair<std::string, std::string>(key,value));
		// std::cout << "Key: [" << key << "] | Value: [" << value << "]" << std::endl;
		it = line_end + 1;  // Move to next line
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