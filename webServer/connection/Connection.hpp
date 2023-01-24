#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "../includes/Define.hpp"
#include "../includes/libraries.hpp"
#include "../InfoFd.hpp"
#include "multiplex/Multiplex.hpp"

class Multiplex;

class Connection : public Multiplex
{
	public:
		std::map<int, InfoServer> m_serverFdMap;
		std::map<int, InfoClient> m_clientFdMap;
		std::map<int, InfoFile> m_fileFdMap;

	private:
		int eventNum;

	public:
		Connection()
		: eventNum(0){
			currEvent = NULL;
		}
	public:
		void eventLoop();
		void handleTimeOut();
		void handleReadEvent();
		void handleWriteEvent();
		void handleErrorEvent();
		void setNonBlock(int fd);
		void initInfoClient(int clientSocket);
		void clearTimeoutedAccess(int socket);
};

#endif
