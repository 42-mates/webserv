#pragma once
# include "Server.hpp"	// config: json
# include <vector>
# include "EventHandler.hpp"

// Manage servers, initialize config and use EventHandler to connect clients
class	Cluster
{
	public:
		Cluster(const JsonValue& json);
		~Cluster();

		Cluster&									start();
		const std::vector<Config*>&					get_configs() const;
		const std::vector<Config*>::const_iterator	get_config_by_host(const std::string &host) const;

		private:
		std::vector<Server*>	_servers;
		std::vector<Config*>	_configs;

		void					clear();
};