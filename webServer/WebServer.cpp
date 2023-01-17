#include "WebServer.hpp"

WebServer::WebServer(Config &config)
{
	std::vector<BaseServer> tmpBase = config.getConfigBase();
	std::vector<BaseServer>::iterator it;
	for (it = tmpBase.begin(); it != tmpBase.end(); ++it)
	{
		InfoServer tmpInfo;

		tmpInfo.m_serverSocket = NONE;
		tmpInfo.m_ipAddress = it.base()->getBServer().host;
		tmpInfo.m_port = it.base()->getBServer().port;
		tmpInfo.m_serverAddr.sin_family = AF_INET; // ip v4
		tmpInfo.m_serverAddr.sin_port = htons(tmpInfo.m_port);
		tmpInfo.m_serverAddr.sin_addr.s_addr = inet_addr(tmpInfo.m_ipAddress.c_str()); // inet_addr converts 'char const *' to 'unsigned long' in network byte order
		tmpInfo.m_serverAddrLen = sizeof(tmpInfo.m_serverAddr);
		memset(tmpInfo.m_serverAddr.sin_zero, 0, sizeof(tmpInfo.m_serverAddr.sin_zero)); // it's a buffer only needed to convert 'sockaddr_in' type to 'sockaddr' type, which is larger type.

		m_serverVector.push_back(tmpInfo);
	}
}

int
WebServer::openServer()
{
	std::vector<InfoServer>::iterator it;
	for (it = m_serverVector.begin(); it != m_serverVector.end(); ++it) {
		it->m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (it->m_serverSocket < 0)
			throw ServerError();

		// Socket resue address option
		int opt = 1;
		if (setsockopt(it->m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == FAIL)
			throw ServerError();

		if (fcntl(it->m_serverSocket, F_SETFL, O_NONBLOCK) == FAIL) {
			std::cerr << "serverSocket fcntl() :";
			return (FAIL);
		}

		if (bind(it->m_serverSocket, (sockaddr *)&it->m_serverAddr, it->m_serverAddrLen) != SUCCESS)
			throw ServerError();

		if (listen(it->m_serverSocket, 128) != SUCCESS)
			throw ServerError();

		m_connection.m_serverFdMap.insert(std::pair<int, InfoServer>(it->m_serverSocket, *it ));
		std::cout << "Listening on " << it->m_ipAddress << ":" << it->m_port << "\n";
	}
	return (SUCCESS);
}

void
WebServer::runServer()
{
	m_connection.declareKqueue();
	std::vector<InfoServer>::iterator it;
	for (it = m_serverVector.begin(); it != m_serverVector.end(); ++it)
	{
		m_connection.enrollEventToChangeList(it->m_serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	}
	m_connection.eventLoop();
}
