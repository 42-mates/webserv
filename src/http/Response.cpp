#include "Response.hpp"
#include "CgiContent.hpp"


Response::Response(Config const &config, int errCode):
	_statusCode(errCode), _contentType("text/html"), _config(config)
{_content = getErrorContent(_statusCode);}

Response::Response(Request const &request, Config const &config):
	_statusCode(200), _contentType("text/html"), _config(config)
{
	// Find the best matching route for this URL
	_routes = find_match(_config, request.getUrl());
	try
	{
		// Check if HTTP method is allowed for this route
		if (!std::count(_routes.method.begin(), _routes.method.end(), request.getMethod()))
			throw(405);  // Method Not Allowed
		// Only support HTTP/1.1
		else if (request.getHttpVersion() != "HTTP/1.1")
			throw(505);  // HTTP Version Not Supported
		// Handle redirections (301 Moved Permanently)
		else if (_routes.is_redirection == true)
			_statusCode = 301;
		// Handle POST requests (file uploads)
		else if (request.getMethod() == "POST")
			_content = getPostContent(request);
		// Handle DELETE requests (file deletion)
		else if (request.getMethod() == "DELETE")
			_content = getDeleteContent(request);
		// Check if this is a CGI request (.py, .php)
		else if (check_extension(request.getUrl()))
			throw(502);  // Bad Gateway (CGI will be handled separately)
		// Default: GET request for static files
		else
			_content = getFileContent(request.getUrl());
	}
	catch(int errCode)
	{
		_statusCode = errCode;
		_content = getErrorContent(_statusCode);
	}
	catch(const std::exception& e)
	{std::cerr << "Some other Error in Default Response: " << e.what() << '\n';}
}

Response::Response(const Request& request, const Config& config, const std::string& cgi_content, CgiResult cgi_res):
	_statusCode(200), _contentType("text/html"), _config(config)
{
	_routes = find_match(_config, request.getUrl());
	try
	{
		if (!std::count(_routes.method.begin(), _routes.method.end(), request.getMethod()))
			throw (405);
		switch (cgi_res)
		{
			case TIMEOUT:
				throw(504);
			case NOTFOUND:
				throw(404);
			case ERROR:
				throw(500);
			default:
				break;
		}
		CgiContent	cgi(cgi_content);
		_extraHeaders = cgi.getHeaders();
		_content = cgi.getBody();
	}
	catch(int errCode)
	{
		_statusCode = errCode;
		_content = getErrorContent(_statusCode);
	}
	catch(const std::exception& e)
	{std::cerr << "Some other errors in CGI Response: " << e.what() << '\n';}
}

Response::~Response() {}

const Config::Route&	Response::find_match(const Config& config, const std::string& url)
{
	std::vector<Config::Route>::const_iterator found = config.get_route().begin();

	for (std::vector<Config::Route>::const_iterator	it = config.get_route().begin(); it != config.get_route().end(); ++it)
		if (url.find(it->path) == 0 && it->path.length() > found->path.length())
			found = it;
	return *found;
}

std::string	Response::generateResponse()
{
	// Build complete HTTP response with headers and body
	std::ostringstream	response;

	// Status line: "HTTP/1.1 200 OK\r\n"
	response << "HTTP/1.1 " << _statusCodes.at(_statusCode) << "\r\n";
	// Add Location header for redirections
	if (_routes.is_redirection)
		response << "Location: " << _routes.redirection << "\r\n";
	// Essential headers
	response << "Content-Type: " << _contentType << "\r\n";
	response << "Content-Length: " << _content.length() << "\r\n";
	response << "Date: " << getCurrentTime(STANDARD) << "\r\n";
	response << "Server: 3GoatServer/1.0\r\n";
	// Close connection for 413 Payload Too Large
	if (_statusCode == 413)
		response << "Connection: close\r\n";
	// else
	// 	response << "Connection: keep-alive\r\n";
	// Add any extra headers from CGI
	if (!_extraHeaders.empty())
		for (std::multimap<std::string, std::string>::iterator	it = _extraHeaders.begin(); it != _extraHeaders.end(); ++it)
			response << it->first << ": " << it->second << "\r\n";
	// Empty line between headers and body
	response << "\r\n";
	// Add response body
	response << _content;
	return response.str();
}

std::string	Response::readFile(const std::string& filename)
{
	std::ifstream	ifs(filename.c_str());
	if (!ifs)
		throw 404;
	std::string		content;
	std::string		line;
	while (std::getline(ifs, line))
		content.append(line + '\n');
	if (ifs.bad())
		throw std::runtime_error("Error occurred while reading the file: " + filename);
	std::string::size_type	dotPos = filename.rfind('.');
	if (dotPos != std::string::npos)
	{
		std::string	ext = toLower(filename.substr(dotPos + 1));
		if (_acceptedFileReversed.count(ext) > 0)
			_contentType = _acceptedFileReversed.at(ext);
	}
	return content;
}

std::string	Response::getErrorContent(int errCode)
{
	std::string	content;

	try
	{
		if (_config.get_error_page().size() != 0)
		{
				const std::string& err_page = _config.get_error_page().at(errCode);
				content = readFile(err_page);
		}
		throw (errCode);
	}
	catch(int)
	{
		// std::cerr << "ERROR PAGES -> CATCHED INT" << '\n';
		content.append("<html><body>");
		content.append("<h2>Oops! Got an error: </h2><h1>");
		content.append(_statusCodes.at(_statusCode));
		content.append("</h1></body></html>\n");
	}
	catch(const std::exception& e)
	{
		std::cerr << "No Error pages: " << e.what() << '\n';
		content.append("<html><body>");
		content.append("<h2>Oops! Got an error: </h2><h1>");
		content.append(_statusCodes.at(_statusCode));
		content.append("</h1></body></html>\n");
	}
	return content;
}

std::string	Response::getFileContent(const std::string& url)
{
	// Convert URL path to actual file path on disk
	std::string	filename;
	std::string	appended;
	std::string	file = url.substr(_routes.path.length());  // Remove route prefix

	// Check if accessing upload directory
	if (file.find("upload/") == 0)
	{
		// Build path: upload_directory + filename
		filename = _routes.upload;
		if (*filename.rbegin() != '/')
			filename += "/";
		filename += file.substr(7);  // Remove "upload/" prefix
	}
	// Otherwise accessing website directory
	else
	{
		// Build path: root_directory + file
		filename = _routes.directory;
		if (file != "/")
		{
			// Avoid double slashes in path
			if (*file.begin() == '/' && *filename.rbegin() == '/')
				filename += file.substr(1);
			else
				filename += file;
		}
	}
	std::string	content;
	try
	{
		if (is_directory(filename))
		{
			std::cout << "-----------DIRECTORY---------" << std::endl;
			try
			{
				appended = filename;
				if (*appended.rbegin() != '/')
					appended += '/';
				appended += _routes.index;
				content = readFile(appended);
			}
			// If the default page index.html is not there, we give the list of directories.
            // If dir_listing is set to false, then it gives 403 Forbidden.
			catch (int)
			{
				if (!_routes.dir_listing)
					throw 403;
				content = directory_listing(filename, url);
			}
			catch (const std::exception &e)
			{
				if (!_routes.dir_listing)
					throw 403;
				content = directory_listing(filename, url);
			}
		}
		else
		{
			std::cout << "-----------REGULAR FILE---------" << std::endl;
			content = readFile(filename);
		}
	}
	catch(int status_code)
	{
		std::cerr << "Error: " << status_code << ", FILE: " << filename << ", appended: " << appended << '\n';
		_statusCode = status_code;
		content = getErrorContent(_statusCode);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error 500: " << e.what() << '\n';
		_statusCode = 500;
		content = getErrorContent(_statusCode);
	}
	return content;
}

std::string	Response::getPostContent(const Request& request)
{
	// Handle POST requests (file uploads)
	std::ostringstream	content;
	std::string			filename;
	std::string			path = _routes.upload;

	// Ensure upload path ends with '/'
	if (*path.rbegin() != '/')
		path.append("/");
	try
	{
		// Get Content-Type from request headers
		_contentType = request.getHeaderValue("Content-Type");	// Throw out_of_range if no content_type
		// Generate unique filename: timestamp + extension based on MIME type
		filename = getCurrentTime(SIMPLE) + check_postFile(_contentType); // throw 403 if not allowed
		path += filename;
		// Create/overwrite file
		std::ofstream	ofs(path.c_str(), std::ofstream::out | std::ofstream::trunc);
		if (!ofs)
			throw (401); // Not authorized to create a file
		// Write request body to file
		ofs << request.getBody();
		_statusCode = 201;  // Created
		content << filename << " is created successfully at [" << _routes.upload << "]\r\n";
	}
	catch(int statusCode)
	{
		_statusCode = statusCode;
		content << getErrorContent(_statusCode);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Other Post Error: " << e.what() << '\n';
		_statusCode = 400;  // Bad Request
		content << getErrorContent(_statusCode);
	}
	return content.str();
}

std::string Response::getDeleteContent(const Request& request)
{
	std::ostringstream	content;
	std::string			path;
	std::string			url = request.getUrl();
	
	// Build the complete file path
	if (url.find("/upload/") == 0)
	{
		std::string relative = url.substr(std::string("/upload").length());
		if (!relative.empty() && relative[0] == '/')
			relative = relative.substr(1);
		path = _routes.upload;
		if (!path.empty() && path[path.length() - 1] != '/')
			path += "/";
		path += relative;
	}
	else
	{
		path = _routes.directory;
		if (!path.empty() && path[path.length() - 1] != '/' && url[0] != '/')
			path += "/";
		path += url;
	}

	try
	{
		// Security: check that path is within authorized directories
		if (path.find("./website") != 0 && path.find("./upload") != 0)
			throw 403; // Forbidden - attempt to access outside authorized zone
		
		// Check that file exists before deletion
		if (access(path.c_str(), F_OK) != 0)
			throw 404; // Not Found - file does not exist
		
		// Check deletion permissions
		if (access(path.c_str(), W_OK) != 0)
			throw 403; // Forbidden - no write permission
		
		// Delete the file
		if (unlink(path.c_str()) != 0)
			throw 500; // Internal Server Error - deletion failed
		
		// Success: return confirmation message
		_statusCode = 204; // No Content (deletion successful)
		std::cout << "[DELETE] Deleted: " << path << std::endl;
		content << "File " << url << " deleted successfully\r\n";
	}
	catch(int statusCode)
	{
		_statusCode = statusCode;
		content << getErrorContent(_statusCode);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Delete Error: " << e.what() << '\n';
		_statusCode = 500;
		content << getErrorContent(_statusCode);
	}
	
	return content.str();
}

// return the iterator of cgi found in the current _routes
std::map<std::string, std::string>::const_iterator	Response::check_cgi(const Config::Route& route, const std::string& url)
{
	std::string				ext;

	if (route.cgi.empty())
		return route.cgi.end();
	std::string::size_type	dotPos = url.rfind('.');
	if (dotPos == std::string::npos)
	{
		if (url == route.path) {
			// if not extension and the client request url == route, then need to check index
			dotPos = route.index.rfind('.');
			if (dotPos != std::string::npos)
			{
				ext = toLower(route.index.substr(dotPos + 1));
				return route.cgi.find(ext);
			}
		}
		return route.cgi.end();
	}
	ext = toLower(url.substr(dotPos + 1));
	return route.cgi.find(ext);
}

std::string	Response::check_postFile(const std::string& type)
{
	// Validate Content-Type and return appropriate file extension
	if (type.empty())
		// Missing Content-Type header in the request
		throw (400); // Bad Request
	if (_acceptedFile.count(type) > 0)  // Check if MIME type is allowed
		return _acceptedFile.at(type);
	else
		throw (403); //Forbidden file type
}

bool	Response::check_extension(const std::string& url)
{
	if (url.empty())
		return false;
	std::string::size_type	dotPos = url.rfind('.');
	if (dotPos == std::string::npos)
		return false;
	std::string				ext = toLower(url.substr(dotPos + 1));
	if (std::count(_allowedCGI.begin(), _allowedCGI.end(), ext) > 0)
		return true;
	return false;
}

std::string	Response::toLower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

std::string	Response::getCurrentTime(Response::TimeForm mode)
{
	time_t		now = time(0);
	struct tm	t_struct = *gmtime(&now);
	char		buff[80];

	if (mode == STANDARD)
		strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S GMT", &t_struct);
	else
		strftime(buff, sizeof(buff), "%F_%H%M%S", &t_struct);
	return std::string(buff);
}

const std::map<int, std::string>	Response::initStatusCodes()
{
	std::map<int, std::string>	tmp;

	tmp[200] = "200 OK";
	tmp[201] = "201 Created";
	tmp[202] = "202 Accepted";
	tmp[204] = "204 No Content";
	tmp[300] = "300 Multiple Choices";
	tmp[301] = "301 Moved Permanently";
	tmp[302] = "302 Found";
	tmp[303] = "303 See Other";
	tmp[400] = "400 Bad Request";
	tmp[401] = "401 Unauthorized";
	tmp[403] = "403 Forbidden";
	tmp[404] = "404 Not Found";
	tmp[405] = "405 Method Not Allowed";
	tmp[413] = "413 Payload Too Large";
	tmp[418] = "418 I'm a teapot";
	tmp[500] = "500 Internal Server Error";
	tmp[502] = "502 Bad Gateway";
	tmp[504] = "504 Gateway Timeout";
	tmp[505] = "505 HTTP Version Not Supported";
	return tmp;
}

const std::vector<std::string>	Response::init_allowed_cgi()
{
	std::vector<std::string>	cgi;

	cgi.push_back("py");
	cgi.push_back("php");
	return cgi;
}

const std::map<std::string, std::string>	Response::initAcceptedFile()
{
	std::map<std::string, std::string>	tmp;

	tmp["application/octet-stream"] = ".bin";
	tmp["application/json"] = ".json";
	tmp["application/pdf"] = ".pdf";
	tmp["application/x-httpd-php"] = ".php";
	tmp["application/x-sh"] = ".sh";
	tmp["application/x-tar"] = ".tar";
	tmp["text/css"] = ".css";
	tmp["text/csv"] = ".csv";
	tmp["text/html"] = ".html";
	tmp["text/plain"] = ".txt";
	tmp["text/javascript"] = ".js";
	tmp["image/gif"] = ".gif";
	tmp["image/jpeg"] = ".jpeg";
	tmp["image/png"] = ".png";
	tmp["audio/mpeg"] = ".mp3";
	tmp["video/mp4"] = ".mp4";
	return tmp;
}

const std::map<std::string, std::string>	Response::initAcceptedFileReversed()
{
	std::map<std::string, std::string>	tmp;

	tmp["bin"] = "application/octet-stream";
	tmp["json"] = "application/json";
	tmp["pdf"] = "application/pdf";
	tmp["php"] = "application/x-httpd-php";
	tmp["sh"] = "application/x-sh";
	tmp["tar"] = "application/x-tar";
	tmp["css"] = "text/css";
	tmp["csv"] = "text/csv";
	tmp["html"] = "text/html";
	tmp["txt"] = "text/plain";
	tmp["js"] = "text/javascript";
	tmp["gif"] = "image/gif";
	tmp["jpeg"] = "image/jpeg";
	tmp["jpg"] = "image/jpeg";
	tmp["png"] = "image/png";
	tmp["mp3"] = "audio/mpeg";
	tmp["mp4"] = "video/mp4";
	return tmp;
}

const std::map<int, std::string>	Response::_statusCodes = Response::initStatusCodes();

const std::vector<std::string>	Response::_allowedCGI = Response::init_allowed_cgi();

const std::map<std::string, std::string>	Response::_acceptedFile = Response::initAcceptedFile();

const std::map<std::string, std::string>	Response::_acceptedFileReversed = Response::initAcceptedFileReversed();