/***************************************************/
/* CODED BY JIN H. BANG ===========================*/
/***************************************************/

#include "webserver/HttpServer.hpp"
#include "configParser/Config.hpp"
#include "webserver/includes/Define.hpp"

int main(int argc, char** argv)
{

	Config config("configFiles/default.conf");

	if (argc == 2) {
		std::string path = argv[1];
		if ((path.compare(path.find('.'), 5, ".conf")) != 0) {
			std::cerr << "Error: Config" << std::endl;
			return (1);
		}
		try {
			config = Config(path);
		}
		catch (std::exception& e) {
			std::cerr << "Error: Config" << std::endl;
			return (1);
		}
	}

	HttpServer webServer(config);

	try {
		webServer.openServer();
	}
	catch (std::exception& e) {
		return (FAIL);
	}

	webServer.runServer(); //runServer() includes Connection class methods

	return (0);
}
