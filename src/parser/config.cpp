#include "config.hpp"

void	Config::check_error_page(int page)
{
	if (page != 400 && page != 404 && page != 405 && page != 500)
		throw Config::BadValue();
}

addrinfo*	Config::ini_addrinfo(const std::string& host, const std::string& port)
{
	struct addrinfo	hints, *r;

	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(host.c_str(), port.c_str(), &hints, &r) != 0)
			throw Config::BadValue("Invalid host in config.\n");
	return r;
}

const std::string&	Config::set(const std::string& value, const std::string& __default)
{
	return set(value, __default, _json);
}

const std::string&	Config::set(const std::string& value, const std::string& __default, const JsonValue& j)
{
	try
	{
		return j[value].as_string();
	}
	catch (const std::exception& e)
	{
		return __default;
	}
}

Config::Config(const JsonValue& j): _addr(NULL), _json(j)
{
	try
	{
		_name = set("name", "server_name");
		_host = set("host", "localhost");
		if (j["port"].as_number() < 0 || j["port"].as_number() > std::numeric_limits<uint16_t>::max())
			throw Config::BadValue();
		_port = j["port"].as_number();
		try
		{_max_body_size = j["max_body"].as_number();}
		catch (const std::exception& e)
		{_max_body_size = 0;}

		try
		{
			for (JsonValue::const_iter_obj	err = j["error"].begin_obj(); err != j["error"].end_obj(); err++)
			{
				check_error_page(std::atoi(err->first.c_str()));
				std::pair<int, std::string>	pair(std::atoi(err->first.c_str()), err->second.as_string());
				_error_pages.insert(pair);
			}
		}
		catch (const Config::BadValue& e)
		{throw Config::BadValue();}
		catch (const std::exception& e) {}

		if (j["routes"].get_arr().size() < 1)
			throw Config::BadValue("Need at least 1 route.\n");
		for (JsonValue::const_iter_arr it_route = j["routes"].begin_arr(); it_route < j["routes"].end_arr(); it_route++)
			_routes.push_back(Route(*it_route));

		_addr = ini_addrinfo(_host, j["port"].as_string());

		std::cout << _name << "\n";
		std::cout << _host << "\n";
		std::cout << _port << "\n";
		std::cout << "--------" << std::endl;
	}
	catch (const std::out_of_range& e)
	{
		throw std::out_of_range("Missing field in config file.\n");
	}
}

Config::~Config()
{
	if (_addr != NULL)
		freeaddrinfo(_addr);
}

Config::Route::Route(): dir_listing(true), is_redirection(false) {}

std::string	Config::handle_directory(const std::string& s)
{
	std::string current = s;
	if (*current.rbegin() != '/')
		current += '/';
	return current;
}

Config::Route::Route(const JsonValue& j): dir_listing(true), is_redirection(false)
{
	path = j["route"].as_string();
	index = set("index", "index.html", j);
	try
	{
		dir_listing = j["dir_listing"].as_bool();
	}
	catch (const std::exception& e)
	{
		dir_listing = true;
	}

	try
	{
		redirection = j["redirection"].as_string();
		is_redirection = true;
	}
	catch (const std::exception& e)
	{
		directory = handle_directory(j["directory"].as_string());
		upload = set("upload", "./upload", j);
		is_redirection = false;
	}

	try
	{
		if (j["method"].get_arr().size() < 1)
			throw Config::BadValue("Need at least 1 method allowed.\n");
		for (JsonValue::const_iter_arr it_method = j["method"].begin_arr(); it_method < j["method"].end_arr(); it_method++)
			method.insert(it_method->as_string());
	}
	catch (const Config::BadValue& e)
	{
		throw Config::BadValue(e.what());
	}
	catch (const std::exception& e)
	{
		method.insert("GET");
		method.insert("POST");
		method.insert("DELETE");
	}

	try
	{
		const JsonValue&	rt = j["cgi"];
		if (rt.get_arr().size() < 1)
			throw Config::BadValue("Not enough cgi's in the array.\n");

		for (JsonValue::const_iter_arr it_cgi = rt.begin_arr(); it_cgi != rt.end_arr(); it_cgi++)
		{
			const JsonValue &c = (*it_cgi);
			if (std::distance(c.begin_obj(), c.end_obj()) != 2)
				throw Config::BadValue("Too many cgi arguments.\n");
			std::pair<std::string, std::string> el(c["extension"].as_string(), c["exec"].as_string());
			if (el.first != "py" && el.first != "php")
				throw Config::BadValue("Invalid extension.\n");
			cgi.insert(el);
		}
	}
	catch (const std::out_of_range& e) {}
	catch (const std::exception& e)
	{
		std::cerr << "CGI: " << e.what() << std::endl;
		throw e;
	}
}

const std::string&	Config::get_name() const
{return _name;}

const std::string&	Config::get_host() const
{return _host;}

uint64_t	Config::get_address() const
{return _address;}

uint64_t	Config::get_max_body_size() const
{return _max_body_size;}

int	Config::get_port() const
{return _port;}

const addrinfo*	Config::get_addr() const
{return _addr;}

const std::map<int, std::string>&	Config::get_error_page() const
{return _error_pages;}

const std::vector<Config::Route>&	Config::get_route() const
{return _routes;}