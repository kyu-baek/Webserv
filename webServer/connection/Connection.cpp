#include "Connection.hpp"

void
Connection::eventLoop()
{
	while (true)
	{
		eventNum = eventMan.senseEvents();
		eventMan.clearChangeList();
		for (int i = 0; i < eventNum; ++i)
		{
			switch (eventMan.currEvent->filter)
			{
			case EVFILT_TIMER:
				handleTimeOut();
				break;
			case EVFILT_READ:
				handleReadEvent();
				break;
			case EVFILT_WRITE:
				handleWriteEvent();
			default:
				if (eventMan.currEvent->flags & EV_ERROR)
					handleErrorEvent();
				break;
			}
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
	if (m_serverFdMap.find(eventMan.currEvent->ident) != m_serverFdMap.end())
	{
		int clientSocket = accept(eventMan.currEvent->ident, \
								(sockaddr *)&m_serverFdMap[eventMan.currEvent->ident].m_serverAddr, \
								&m_serverFdMap[eventMan.currEvent->ident].m_serverAddrLen);
		if (clientSocket == FAIL)
			std::cerr << "	ERROR : accept() in Server Event Case\n";
		setNonBlock(clientSocket);
		eventMan.enrollEventToChangeList(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		eventMan.enrollEventToChangeList(clientSocket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER, NULL);
		initInfoClient(clientSocket);
	}

	/* Client Event Case */
	if (m_clientFdMap.find(eventMan.currEvent->ident) != m_clientFdMap.end())
	{
		char buffer[BUFFER_SIZE + 1] = {0,};

		ssize_t valRead = read(eventMan.currEvent->ident, buffer, BUFFER_SIZE);
		if (valRead == FAIL)
		{
			std::cerr << "	ERROR : read() in Client Event Case\n";
			std::vector<int>::iterator it;
			for (it = m_serverFdMap[m_clientFdMap[eventMan.currEvent->ident].m_server->m_serverSocket].m_clients.begin(); it != m_serverFdMap[m_clientFdMap[eventMan.currEvent->ident].m_server->m_serverSocket].m_clients.end(); ++it)
			{
				if (*it == (int)eventMan.currEvent->ident)
					break ;
			}
			if (it != m_serverFdMap[m_clientFdMap[eventMan.currEvent->ident].m_server->m_serverSocket].m_clients.end())
			{
				m_serverFdMap[m_clientFdMap[eventMan.currEvent->ident].m_server->m_serverSocket].m_clients.at(eventMan.currEvent->ident);
			}
			close(eventMan.currEvent->ident);
			m_clientFdMap.erase(eventMan.currEvent->ident);
		}
		else if (valRead > 0)
		{
			buffer[valRead] = '\0';
			m_clientFdMap[eventMan.currEvent->ident].reqParser.makeRequest(buffer);
		}
		else if (valRead == 0)
		{
			if (m_clientFdMap[eventMan.currEvent->ident].reqParser.t_result.pStatus == Request::ParseComplete)
			{

			}
			if (m_clientFdMap[eventMan.currEvent->ident].reqParser.t_result.pStatus == Request::ParseError)
			{

			}
		}
	}
}

void
Connection::handleWriteEvent()
{

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
	m_serverFdMap[eventMan.currEvent->ident].m_clients.push_back(clientSocket);

	InfoClient tmpInfo;
	tmpInfo.m_socketFd = clientSocket;
	tmpInfo.m_server = &m_serverFdMap[eventMan.currEvent->ident];
	m_clientFdMap.insert(std::pair<int, InfoClient>(clientSocket, tmpInfo));
}
