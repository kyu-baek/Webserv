#ifndef SERVER_HPP
#define SERVER_HPP

#include "../includes/libraries.hpp"
#include "../includes/Define.hpp"

#include "../config/configParser/Config_struct.hpp"

class Server
{
	public:
		int m_serverFd;
		std::string m_ipAddress;
		int m_port;
		struct sockaddr_in m_serverAddr;
		unsigned int m_serverAddrLen;
		std::string m_requestMsg;
		std::vector<int> m_clientVec;
		int maxRequestBodySize;

	public:
		std::map<std::string, std::vector<int> > m_errorPages;
		std::map<std::string, Location> m_location;
		std::map<std::string, CgiConfig> m_cgi;

	public:
		Server &operator=(Server const &rhs)
		{
			m_serverFd = rhs.m_serverFd;
			m_ipAddress = rhs.m_ipAddress;
			m_port = rhs.m_port;
			m_serverAddr = rhs.m_serverAddr;
			m_serverAddrLen = rhs.m_serverAddrLen;
			m_requestMsg = rhs.m_requestMsg;
			maxRequestBodySize = rhs.maxRequestBodySize;
			return (*this);
		}
};

#endif
