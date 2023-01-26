#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "../includes/Define.hpp"
#include "../includes/libraries.hpp"
#include "../server/Server.hpp"
#include "../client/Client.hpp"
#include "../multiplex/Multiplex.hpp"

class Multiplex;

class Connection : public Multiplex
{
	public:
		std::map<int, Server> m_serverMap;
		std::map<int, Client> m_clientMap;
		std::map<int, Client*> m_fileMap;

	private:
		int eventNum;

	public:
		Connection()
		: eventNum(0){
			currEvent = NULL;
		}

	public:
		void eventLoop();
		void handleEofEvent();
		void handleTimeOut();
		void handleReadEvent();
		void handleWriteEvent();
		void handleErrorEvent();
		void setNonBlock(int fd);
		void initClient(int clientSocket);
		void deleteClient(int socket);
};

#endif
