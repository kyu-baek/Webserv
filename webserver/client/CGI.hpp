#ifndef CGI_HPP
#define CGI_HPP

#include "../includes/libraries.hpp"
#include "../includes/Define.hpp"

class CGI {
	public:
		std::map<std::string, std::string> envMap;

	public:
		void initEnvMap()
		{
			envMap.insert(std::pair<std::string, std::string>("AUTH_TYPE", ""));
			envMap.insert(std::pair<std::string, std::string>("GATE_INTERFACE", "CGI/1.1"));
			//envMap.insert(std::pair<std::string, std::string>("SERVER_NAME", "webserver"));
			envMap.insert(std::pair<std::string, std::string>("SERVER_PROTOCOL", "HTTP/1.1"));
			envMap.insert(std::pair<std::string, std::string>("REMOTE_USER", ""));
			envMap.insert(std::pair<std::string, std::string>("CONTENT_LENGTH", "-1"));
			envMap.insert(std::pair<std::string, std::string>("CONTENT_TYPE", ""));
		}
};
#endif
