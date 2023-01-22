#ifndef CGI_HPP
#define CGI_HPP

#include "../../includes/Define.hpp"
#include "../../includes/libraries.hpp"
#include "../../InfoFd.hpp"

//class InfoClient;

class CGI {
	public:
		std::map<std::string, std::string> envMap;

	public:
		void initEnvMap(InfoClient *infoClient)
		{
			envMap.insert(std::pair<std::string, std::string>("AUTH_TYPE", ""));
			envMap.insert(std::pair<std::string, std::string>("GATE_INTERFACE", "CGI/1.1"));
			envMap.insert(std::pair<std::string, std::string>("SERVER_NAME", "webserver"));
			envMap.insert(std::pair<std::string, std::string>("SERVER_PROTOCOL", "HTTP/1.1"));
			envMap.insert(std::pair<std::string, std::string>("REMOTE_USER", ""));
			envMap.insert(std::pair<std::string, std::string>("CONTENT_LENGTH", "-1"));
			envMap.insert(std::pair<std::string, std::string>("CONTENT_TYPE", ""));
			if (infoClient->reqParser.t_result.method == GET)
				envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "GET"));
			else if (infoClient->reqParser.t_result.method == POST)
				envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "POST"));
			else if (infoClient->reqParser.t_result.method == DELETE)
				envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "DELETE"));
			envMap.insert(std::pair<std::string, std::string>("QUERY_STRING", infoClient->reqParser.t_result.query ));
			envMap.insert(std::pair<std::string, std::string>("SERVER_PORT", std::to_string(infoClient->m_server->m_port)));
		}
};

#endif
