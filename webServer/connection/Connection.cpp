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
			else if (currEvent->flags & EV_ERROR || currEvent->flags & EV_EOF)
				handleErrorEvent();
		}
	}
}

void
Connection::handleTimeOut()
{
	std::cout << "\n\n EVFILT_TIMER : " << currEvent->ident << "\n";
	if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
	{
		clearTimeoutedAccess(currEvent->ident);
	}
	std::cout << "\n\n TIMER EVENT DONE-------------------------------------\n";
}

void
Connection::clearTimeoutedAccess(int socket)
{
	std::cout << "clearTimeoutedAccess" << std::endl;
	if (m_clientFdMap.find(socket) == m_clientFdMap.end())
		return ;
	std::map <int, InfoFile>::iterator it;
	for (it = m_fileFdMap.begin(); it != m_fileFdMap.end(); it++)
	{
		if (it->second.p_infoClient->m_socketFd == (int)socket)
		{
			m_fileFdMap.erase(it->first);
			continue;
		}
	}
	//Delete for malloc or connect map!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!; 
	m_clientFdMap.erase(socket);
	enrollEventToChangeList(socket, EVFILT_TIMER, EV_DELETE | EV_DISABLE, 0, 0, NULL);
	close(socket);
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

		ssize_t valRead = recv(currEvent->ident, buffer, BUFFER_SIZE, 0);
		if (valRead == FAIL || valRead == 0)
		{
			std::cerr << "	ERROR : read() in Client Event Case\n";
			std::vector<int>::iterator it;
			for (it = m_serverFdMap[m_clientFdMap[currEvent->ident].m_server->m_serverSocket].m_clients.begin(); it != m_serverFdMap[m_clientFdMap[currEvent->ident].m_server->m_serverSocket].m_clients.end(); ++it)
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
		else
		{
			buffer[valRead] = '\0';
			m_clientFdMap[currEvent->ident].reqParser.makeRequest(buffer);
	
			if (m_clientFdMap[currEvent->ident].reqParser.t_result.pStatus == Request::ParseComplete)
			{
				enrollEventToChangeList(currEvent->ident, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
				std::cout << "\n-----------------\n";
				m_clientFdMap[currEvent->ident].reqParser.printRequest();
				std::cout << "-----------------\n\n";
				
				int fileFd = m_clientFdMap[currEvent->ident].m_responser->openResponse();
				if (m_clientFdMap[currEvent->ident].reqParser.t_result.method == GET)
				{
					fcntl(fileFd, F_SETFL, O_NONBLOCK);
					enrollEventToChangeList(fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					InfoFile tmpInfo;
					tmpInfo.p_infoClient = &m_clientFdMap[currEvent->ident];
					m_fileFdMap.insert(std::make_pair(fileFd, tmpInfo));
				}
				else
				{
					fcntl(fileFd, F_SETFL, O_NONBLOCK);
					enrollEventToChangeList(fileFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					InfoFile tmpInfo;
					tmpInfo.p_infoClient = &m_clientFdMap[currEvent->ident];
					m_fileFdMap.insert(std::make_pair(fileFd, tmpInfo));
				}
				// if (GET)
				// {
				// 	int fileFd = openStaticHtml(target);
				// 	m_fileFdMap.insert();
				// 	enrollEventToChangeList(fileFd, EVFILT_READ)~~;
				// 	enrollEventToChangeList(currEvent->ident, REMOVE);
				// }
				// if (reqParser ~~ == POST)
				// {

				// }
				// if (DELETE)
				// {

				// }
			}
			if (m_clientFdMap[currEvent->ident].reqParser.t_result.pStatus == Request::ParseError)
			{
			}
		}
	}


	if (m_fileFdMap.find(currEvent->ident) != m_fileFdMap.end())
	{
		std::cout << "File Read Event : " << currEvent->ident << std::endl;
		if (m_fileFdMap[currEvent->ident].p_infoClient->m_responser->isCgiIng == CGIFL::None || 
			m_fileFdMap[currEvent->ident].p_infoClient->m_responser->isCgiIng == CGIFL::Complete)
		{
			int res =  m_fileFdMap[currEvent->ident].p_infoClient->m_responser->readFile(currEvent->ident);

			switch (res)
			{
			case File::Error:
				// 500 error page open
				// std::cout << "fError" << std::endl;
				break;
			case File::Making:
				// keep reading
				//	//std::cout << "fMaking = " << _clientMap[_fdMap[currEvent->ident]].file.buffer << std::endl;
				break;
			case File::Complete:
				enrollEventToChangeList(m_fileFdMap[currEvent->ident].p_infoClient->m_socketFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				m_fileFdMap[currEvent->ident].p_infoClient->m_responser->startResponse();
				m_fileFdMap.erase(m_fileFdMap.find(currEvent->ident)->first);
				close(currEvent->ident);
				// if (m_fileFdMap[currEvent->ident].p_infoClient->m_responser->isCgiIng == false)
				// {
				// 	enrollEventToChangeList(m_fileFdMap[currEvent->ident].p_infoClient->m_socketFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				// 	m_fileFdMap[currEvent->ident].p_infoClient->m_responser->startResponse();
				// 	m_fileFdMap.erase(m_fileFdMap.find(currEvent->ident)->first);
				// 	close(currEvent->ident);
				// }
				// else
				// {
				// 	// std::cout << "cgi!!!!!!\n";
				// 	//m_fileFdMap[currEvent->ident].p_infoClient->m_responser->m_file.size =
				// 	enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				// }
				//_eventManager.enrollEventToChangeList(_responserMap[_fdMap[currEvent->ident]].fds[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				break;
			}
		}
	}
}


void
Connection::handleWriteEvent()
{
	if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
	{
		std::cout << " CLIENT FD write\n";
		int res =  m_clientFdMap[currEvent->ident].m_responser->writeClient(currEvent->ident);

		switch (res)
		{
		case Send::Error:
			//m_clientFdMap[currEvent->ident].clear();
			m_clientFdMap.erase(currEvent->ident);
			close(currEvent->ident);
			break;
		case Send::Making:
			//	//std::cout << "Send Making = " << _clientMap[_fdMap[currEvent->ident]].file.buffer << std::endl;
			break;
		case Send::Complete:
			enrollEventToChangeList(currEvent->ident, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
			enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, NULL);
			m_clientFdMap[currEvent->ident].m_responser->clearFileEvent();
			// close(currEvent->ident);
			// m_clientFdMap.erase(currEvent->ident);

			// m_clientFdMap[currEvent->ident].m_responser->clearFileEvent();
			//m_clientFdMap[currEvent->ident].clear();
			break;
		}
	}

	if (m_fileFdMap.find(currEvent->ident) != m_fileFdMap.end())
	{
		std::cout << " FILE FD write\n";
		int res =  m_clientFdMap[currEvent->ident].m_responser->writePipe(currEvent->ident);

		switch (res)
		{
		case Write::Error:
			//m_clientFdMap[currEvent->ident].clear();
			m_clientFdMap.erase(currEvent->ident);
			close(currEvent->ident);
			break;
		case Write::Making:
			//	//std::cout << "Send Making = " << _clientMap[_fdMap[currEvent->ident]].file.buffer << std::endl;
			break;
		case Write::Complete:
			close(currEvent->ident);
			m_fileFdMap.erase(currEvent->ident);
			break;
		}
	}
}

void
Connection::handleErrorEvent()
{
	if (m_serverFdMap.find(currEvent->ident) != m_serverFdMap.end())
	{
		this->m_serverFdMap.erase(this->m_serverFdMap.find(currEvent->ident));
		close(currEvent->ident);
	}
	else if (this->m_clientFdMap.find(currEvent->ident) != this->m_clientFdMap.end())
	{
		this->m_clientFdMap.erase(this->m_clientFdMap.find(currEvent->ident));
		close(currEvent->ident);
	}
	this->m_fileFdMap.erase(this->m_fileFdMap.find(currEvent->ident));
	close(currEvent->ident);
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
	tmpInfo.m_responser = new Response(); //delete needed
	m_clientFdMap.insert(std::pair<int, InfoClient>(clientSocket, tmpInfo));
	m_clientFdMap[clientSocket].m_responser->p_infoClient = &m_clientFdMap[clientSocket];
	m_clientFdMap[clientSocket].m_responser->isCgiIng = CGIFL::None;
}
