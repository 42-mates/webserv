#include <iostream>
#include <exception>
#include <csignal>
#include "utils.hpp"
#include "parser.hpp"
#include "json.hpp"
#include "cluster.hpp"

void	handle_sigint(int _sig) {throw (int)_sig;}

int	main(int ac, char** av)
{
	if (ac != 2)
		return 1;
	if (end_with_json(av[1]) == false || is_directory(av[1]))
	{
		std::cerr << "Error: Configuration must be a json file." << std::endl;
		return 1;
	}
	std::ifstream	input(av[1]);
	if (!input)
		return 1;
	try
	{
		signal(SIGINT, handle_sigint);
		signal(SIGPIPE, SIG_IGN);
		JsonValue	json = JsonParser::parse(input);
		std::cout << json << std::endl;
		Cluster	cluster(json);

		cluster.start();
	}
	catch (int v)
	{
		if (v == 1)
			return v;
		std::cerr << "\n("<< v << ") Exit successful." << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what();
	}
	return 0;
}

// 1. ./webserv config/config.json
// 	This terminal will show the server log.
// 	Need to open another terminal to use.
// 2. ./webserv config/config.json &
// 	Run in background. Can directly use the terminal.