#include "Connection.hpp"

void
Connection::eventLoop()
{
	while (true)
	{
		eventNum = senseEvents();
		clearChangeList();
		for (int i = 0; i < eventNum; ++i)
		{
			currEvent = &getEventList()[i];
			if (currEvent->filter == EVFILT_TIMER)
				handleTimeOut();
			else if (currEvent->filter == EVFILT_READ)
				handleReadEvent();
			else if (currEvent->filter == EVFILT_WRITE)
				handleWriteEvent();
			else if (currEvent->flags & EV_ERROR)
				handleErrorEvent();
		}
	}
}

void
Connection::handleTimeOut()
{

}

void
Connection::handleReadEvent()
{
	/* Server Event Case */
	if (m_serverFdMap.find(currEvent->ident) != m_serverFdMap.end())
	{
		int clientSocket = accept(currEvent->ident, \
								(sockaddr *)&m_serverFdMap[currEvent->ident].m_serverAddr, \
								&m_serverFdMap[currEvent->ident].m_serverAddrLen);
		if (clientSocket == FAIL)
			std::cerr << "	ERROR : accept() in Server Event Case\n";
		setNonBlock(clientSocket);
		enrollEventToChangeList(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		enrollEventToChangeList(clientSocket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER, NULL);
		initInfoClient(clientSocket);
	}

	/* Client Event Case */
	if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
	{
		char buffer[BUFFER_SIZE + 1] = {0,};

		ssize_t valRead = read(currEvent->ident, buffer, BUFFER_SIZE);
		if (valRead == FAIL)
		{
			std::cerr << "	ERROR : read() in Client Event Case\n";
			std::vector<int>::iterator it;
			for (it = m_serverFdMap[m_clientFdMap[currEvent->ident].m_server->m_serverSocket].m_clients.begin();\
				it != m_serverFdMap[m_clientFdMap[currEvent->ident].m_server->m_serverSocket].m_clients.end(); ++it)
			{
				if (*it == (int)currEvent->ident)
					break ;
			}
			if (it != m_serverFdMap[m_clientFdMap[currEvent->ident].m_server->m_serverSocket].m_clients.end())
			{
				m_serverFdMap[m_clientFdMap[currEvent->ident].m_server->m_serverSocket].m_clients.at(currEvent->ident);
			}
			close(currEvent->ident);
			m_clientFdMap.erase(currEvent->ident);
		}
		else if (valRead > 0)
		{
			buffer[valRead] = '\0';
			m_clientFdMap[currEvent->ident].reqParser.makeRequest(buffer);
		}
		else if (valRead == 0)
		{
			m_clientFdMap[currEvent->ident].reqParser.printRequest();
			if (m_clientFdMap[currEvent->ident].reqParser.t_result.pStatus == Request::ParseComplete)
			{
				if (m_clientFdMap[currEvent->ident].status == ResNone)
				{
					m_clientFdMap[currEvent->ident].m_responserPtr->openResponse();
					m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->m_fileFdMapPtr = &m_fileFdMap;
					int fileFd = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_file.fd;
					enrollEventToChangeList(fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					fcntl(fileFd, F_SETFL, O_NONBLOCK);
					m_fileFdMap.insert(std::make_pair(m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_file.fd, *(m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr)));
				}
			}
			if (m_clientFdMap[currEvent->ident].reqParser.t_result.pStatus == Request::ParseError)
			{
				std::cerr << "	Error : parse\n";
				// 404
			}
		}
	}

	/* File Event Case */
	if (m_fileFdMap.find(currEvent->ident) != m_fileFdMap.end())
	{
		int res = m_fileFdMap[currEvent->ident].m_infoClientPtr->m_responserPtr->m_fileManagerPtr->readFile(currEvent->ident);
		// std::string bodyContents += readBuffer();
		// std::string header = makeHeader();
		// std::string fullRes = header + bodyContents;
		switch (res)
		{
		case FileError:
			// 500 error page open
			// std::cout << "fError" << std::endl;
			break;
		case FileMaking:
			// keep reading
			//	//std::cout << "fMaking = " << _clientMap[_fdMap[currEvent->ident]].file.buffer << std::endl;
			break;
		case FileComplete:
			if (m_fileFdMap[currEvent->ident].isCgi == false)
			{
				m_fileFdMap[currEvent->ident].m_infoClientPtr->m_responserPtr->startResponse();
				enrollEventToChangeList(m_fileFdMap[currEvent->ident].m_infoClientPtr->m_socketFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				// std::cout << currEvent->ident << " file reading done. open client " << _fdMap[currEvent->ident] << std::endl;;
				close(currEvent->ident);
				m_fileFdMap.erase(m_fileFdMap.find(currEvent->ident)->first);
			}
			else
			{
				// std::cout << "cgi!!!!!!\n";
				enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
			}
			//_eventManager.enrollEventToChangeList(_responserMap[_fdMap[currEvent->ident]].fds[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);

			break;
		}
	}
}

void
Connection::handleWriteEvent()
{
	//
}

void
Connection::handleErrorEvent()
{

}

void
Connection::setNonBlock(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == FAIL)
		std::cerr << "	ERROR : fcntl()";
}

void
Connection::initInfoClient(int clientSocket)
{
	m_serverFdMap[currEvent->ident].m_clients.push_back(clientSocket);

	InfoClient tmpInfo;
	tmpInfo.m_socketFd = clientSocket;
	tmpInfo.m_server = &m_serverFdMap[currEvent->ident];
	tmpInfo.m_responserPtr = new Response(); //delete needed
	tmpInfo.m_responserPtr->m_fileManagerPtr = new FileManage(); // delete needed
	m_clientFdMap.insert(std::pair<int, InfoClient>(clientSocket, tmpInfo));
	m_clientFdMap[clientSocket].m_responserPtr->m_infoClientPtr = &m_clientFdMap[clientSocket];
}
