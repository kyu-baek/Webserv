#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "includes/Define.hpp"
#include "includes/libraries.hpp"
#include "config/configParser/Config.hpp"
#include "connection/Connection.hpp"

class WebServer {
	public:
		std::vector<Server> m_serverVector;
		Connection m_connection;

	public:
		WebServer(Config &config);
		// ~WebServer();
		int openServer();
		void runServer();
		void closeServer();

	public:
		class ServerError : public std::exception
		{
			public:
				const char *what() const throw()
				{
					return ("Connection Error");
				}
		};
};

#endif
