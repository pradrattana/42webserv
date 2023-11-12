#include "Server.hpp"
#include "ConfigParser.hpp"
#include <iostream>

int main(int argc, char **argv, char **env)
{
	(void)argc;
	(void)argv;
	(void)env;

	if (argc == 2)
	{
		ConfigParser conf;

		try
		{
			conf.readConfig(argv[1]);
			// conf.printAll();
		}
		catch (const std::exception &e)
		{
			std::cout << "Error: " << e.what() << std::endl;
			return 1;
		}

		Server server(conf);
		(void)server;
	}

	return 0;
}
