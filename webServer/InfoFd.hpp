#ifndef INFOFD_HPP
#define INFOFD_HPP

#include "includes/libraries.hpp"
#include "includes/Define.hpp"
#include "connection/request/Request.hpp"
#include "config/configParser/Config_struct.hpp"
#include "connection/response/Response.hpp"
#include "connection/fileManage/FileManage.hpp"

class Request;
class Response;
class InfoClient;
class InfoServer;
class InfoFile;
class FileManage;

class InfoClient
{
	public:
		int m_socketFd;
		InfoServer *m_server;
		Request reqParser;
		Response *m_responserPtr;
		bool isCgi;

	public:
		int status;

	public:
		InfoClient()
		: m_socketFd(-1), m_server(NULL), status(0) {}
};

class InfoServer
{
	public:
		int m_serverSocket;
		std::string m_ipAddress;
		int m_port;
		struct sockaddr_in m_serverAddr;
		unsigned int m_serverAddrLen;
		std::string m_requestMsg;
		std::vector<int> m_clients;

	public:
		std::map<std::string, std::vector<int> > m_errorPages;
		std::map<std::string, Location> m_location;
		std::map<std::string, CgiConfig> m_cgi;

	public:
		InfoServer &operator=(InfoServer const &rhs)
		{
			m_serverSocket = rhs.m_serverSocket;
			m_ipAddress = rhs.m_ipAddress;
			m_port = rhs.m_port;
			m_serverAddr = rhs.m_serverAddr;
			m_serverAddrLen = rhs.m_serverAddrLen;
			m_requestMsg = rhs.m_requestMsg;
			return (*this);
		}
};

class InfoFile
{
	public:
		InfoClient *m_infoClientPtr;
		FileManage *m_fileManagerPtr;
		std::string srcPath;
		std::map<int, InfoFile> *m_fileFdMapPtr;
		int inFds[2];
		int outFds[2];
		bool isCgi;
		int isFile;
};


#endif
