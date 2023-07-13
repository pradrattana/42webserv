#include "ConfigParser.hpp"
#include <iostream>

int	main() {

	try {
		/* code */
		ConfigParser	p("prrattan.42.fr.conf");
		(void)p;
		p.printAll();
	} catch(const std::exception& e) {
		std::cout << "what(): " << e.what() << std::endl;
		return 1;
	}
	
	

	// Server	server;
	// (void)server;

	return 0;
}
