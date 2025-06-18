#include "cluster.hpp"

Cluster::Cluster(const JsonValue& json)
{
	try
	{
		if (json.get_arr().size() < 1)
			throw Config::BadValue("Not enough configs.\n");

		std::set<int>	used_ports;
		for (JsonValue::const_iter_arr it = json.begin_arr(); it < json.end_arr(); it++)
		{
			// Port duplicate check
			int	port = (*it)["port"].as_number();
			if (!used_ports.insert(port).second)
			{
				std::stringstream	ss;
				ss << "Duplicate port: " << port << "." << std::endl;
				throw Config::BadValue(ss.str());
			}

			Config	*current = new Config(*it);
			_configs.push_back(current);
			_servers.push_back(new Server(*current));
		}
	}
	catch (const std::exception& e)
	{
		Cluster::clear();
		throw std::runtime_error(e.what());
	}
}

Cluster&	Cluster::start()
{
	EventHandler	e(*this);
	for (std::vector<Server*>::iterator it = _servers.begin(); it < _servers.end(); it++)
		e.addServer(*(*it));
	e.epollLoop();
	return *this;
}

void	Cluster::clear()
{
	for (std::vector<Server*>::iterator	it = _servers.begin(); it < _servers.end(); it++)
		if (*it != NULL)
			delete *it;
	for (std::vector<Config*>::iterator	it = _configs.begin(); it < _configs.end(); it++)
		if (*it != NULL)
			delete *it;
}

Cluster::~Cluster()
{Cluster::clear();}

const std::vector<Config*>&	Cluster::get_configs() const
{return _configs;}

// Returns an iterator of config, if it can't be found it returns `end()`
const std::vector<Config*>::const_iterator	Cluster::get_config_by_host(const std::string& host) const
{
	std::stringstream	s;
	for (std::vector<Config*>::const_iterator	it = _configs.begin(); it < _configs.end(); it++)
	{
		s << (*it)->get_host() << ':' << (*it)->get_port();
		if (host == s.str())
		{
			std::cerr << "FOUND: [" << s.str() << "], actual host: [" << host << ']' << std::endl;
			return it;
		}
		s.str("");
	}
	return _configs.end();
}