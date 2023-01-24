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
		if (it->second.m_infoClientPtr->m_socketFd == (int)socket)
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

		if (m_serverFdMap.empty() == true)
		{
			int clientSocket = accept(currEvent->ident,
									  (sockaddr *)&m_serverFdMap[currEvent->ident].m_serverAddr,
									  &m_serverFdMap[currEvent->ident].m_serverAddrLen);
			if (clientSocket == FAIL)
				std::cerr << "  ERROR : accept() in Server Event Case\n";
			setNonBlock(clientSocket);
			enrollEventToChangeList(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
			enrollEventToChangeList(clientSocket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER, NULL);
			initInfoClient(clientSocket);
		}
		else
		{
			std::vector<int>::iterator it;
			for (it = m_serverFdMap[currEvent->ident].m_clients.begin(); it != m_serverFdMap[currEvent->ident].m_clients.end(); ++it)
			{
				if (it != m_serverFdMap[currEvent->ident].m_clients.end() && m_clientFdMap[*it].status == Res::Complete)
					break;
			}
			if (it == m_serverFdMap[currEvent->ident].m_clients.end())
			{
				int clientSocket = accept(currEvent->ident,
										  (sockaddr *)&m_serverFdMap[currEvent->ident].m_serverAddr,
										  &m_serverFdMap[currEvent->ident].m_serverAddrLen);
				if (clientSocket == FAIL)
					std::cerr << "  ERROR : accept() in Server Event Case\n";
				setNonBlock(clientSocket);
				enrollEventToChangeList(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
				enrollEventToChangeList(clientSocket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER, NULL);
				initInfoClient(clientSocket);
			}
		}
	}

	/* Client Event Case */
	if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
	{
		char buffer[BUFFER_SIZE + 1] = {0, };

		ssize_t valRead = read(currEvent->ident, buffer, BUFFER_SIZE);

		if (valRead == FAIL)
		{
			std::cerr << "	ERROR : read() in Client Event Case\n";
			std::vector<int>::iterator it;
			for (it = m_serverFdMap[m_clientFdMap[currEvent->ident].m_server->m_serverSocket].m_clients.begin();
					it != m_serverFdMap[m_clientFdMap[currEvent->ident].m_server->m_serverSocket].m_clients.end(); ++it)
			{
				if (*it == (int)currEvent->ident)
					break;
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
			m_clientFdMap[currEvent->ident].status = Res::None;
		}
		if (valRead < BUFFER_SIZE)
		{
			if (m_clientFdMap[currEvent->ident].reqParser.t_result.pStatus == Request::ParseComplete)
			{
				if (m_clientFdMap[currEvent->ident].status == Res::None)
				{
					// m_clientFdMap[currEvent->ident].reqParser.printRequest();
					std::cout << "	--REQUEST FROM CLIENT " << currEvent->ident << "--\n"
								<< m_clientFdMap[currEvent->ident].reqParser.t_result.orig << "\n\n";
					m_clientFdMap[currEvent->ident].reqParser.t_result.orig = "";

					m_clientFdMap[currEvent->ident].m_responserPtr->openResponse();
					if (m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_file.fd != -1)
					{
						if (m_clientFdMap[currEvent->ident].isCgi == false)
						{
							int fileFd = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_file.fd;
							enrollEventToChangeList(currEvent->ident, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
							enrollEventToChangeList(fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							fcntl(fileFd, F_SETFL, O_NONBLOCK);
							m_fileFdMap.insert(std::make_pair(fileFd, *(m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr)));
							m_fileFdMap[fileFd].m_fileManagerPtr = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr;
							m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->m_fileFdMapPtr = &m_fileFdMap;
						}
						else
						{
							int fileFd = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_file.fd;
							enrollEventToChangeList(fileFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
							fcntl(fileFd, F_SETFL, O_NONBLOCK);
							m_fileFdMap.insert(std::make_pair(fileFd, *(m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr)));
							m_fileFdMap[fileFd].m_fileManagerPtr = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr;
							m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->m_fileFdMapPtr = &m_fileFdMap;
						}
					}
				}
				// else if (Send::Making)
				// {

				// }
				// else if (Send::Complete)
				// {
				// 	enrollEventToChangeList(currEvent->ident, EVFI)

				// }
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
		if (m_fileFdMap[currEvent->ident].m_infoClientPtr->status == Res::Making)
		{

			//	std::cout << "File Event Case : " << currEvent->ident <<std::endl;
			int res = m_fileFdMap[currEvent->ident].m_fileManagerPtr->readFile(currEvent->ident);
			// std::string bodyContents += readBuffer();
			// std::string header = makeHeader();
			// std::string fullRes = header + bodyContents;
			switch (res)
			{
			case File::Error:
				// 500 error page open
				std::cout << "fError" << std::endl;
				break;
			case File::Making:
				// std::cout << "fMaking = " << std::endl;
				break;
			case File::Complete:
				std::cout << "Complete" << std::endl;
				enrollEventToChangeList(currEvent->ident, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
				m_fileFdMap[currEvent->ident].m_infoClientPtr->status = Res::Complete;
				if (m_fileFdMap[currEvent->ident].isCgi == false)
				{
					std::cout << "start\n\n";
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
}

void
Connection::handleWriteEvent()
{
	/* write event */
	if (currEvent->filter == EVFILT_WRITE)
	{
		// std::cout << "\n\n WRITE EVENT : " << currEvent->ident << std::endl;
		
		if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
		{
			int result;
			if (m_clientFdMap[currEvent->ident].isCgi == false)
				result = m_clientFdMap[currEvent->ident].m_responserPtr->sendResponse();
			if (m_clientFdMap[currEvent->ident].isCgi == true)
			{
				if (m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->isCgiOutDone() == true)
				{
					m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->srcPath = m_clientFdMap[currEvent->ident].m_responserPtr->cgiOutPath;
					result = m_clientFdMap[currEvent->ident].m_responserPtr->sendResponse();
				}
			}

			switch (result)
			{
			case Send::Error:
				// 500 error page open
				// std::cout << "fError" << std::endl;
				m_clientFdMap[currEvent->ident].status = Res::None;
				break;
			case Send::Making:
				// keep reading
				//	//std::cout << "fMaking = " << m_fileFdMap[_fdMap[currEvent->ident]].file.buffer << std::endl;
				m_clientFdMap[currEvent->ident].status = Res::Making;
				break;
			case Send::Complete:
					std::cout << "	--RESPONSE SENT TO CLIENT " << currEvent->ident << "--\n\n";

					enrollEventToChangeList(currEvent->ident, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, NULL);
					m_clientFdMap[currEvent->ident].m_responserPtr->clearResInfo();
					m_clientFdMap[currEvent->ident].m_responserPtr->clearResponseByte();
					m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->clearFileEvent();
					m_clientFdMap[currEvent->ident].status = Res::Complete;
					m_clientFdMap[currEvent->ident].reqParser.clearRequest();
					if (m_clientFdMap[currEvent->ident].isCgi == true)
					{
						m_clientFdMap[currEvent->ident].isCgi = false;
						m_clientFdMap[currEvent->ident].m_responserPtr->cgiOutPath = "";
						m_clientFdMap[currEvent->ident].m_responserPtr->cgiOutTarget = "";
					}
					//make clear client info logic

					// close(currEvent->ident); //temporarily added
					// // m_clientFdMap[currEvent->ident].m_server->m_clients.erase();
					// m_clientFdMap.erase(currEvent->ident); //temporarily added
				break;
			}

			if (m_fileFdMap.find(currEvent->ident) != m_fileFdMap.end() && m_fileFdMap[currEvent->ident].m_infoClientPtr->isCgi == true)
			{
				int result = m_fileFdMap[currEvent->ident].m_infoClientPtr->m_responserPtr->m_fileManagerPtr->writePipe(currEvent->ident);
				std::cout << "		RESULT OF CGI WRITE : " << result << "\n\n";
				switch (result)
				{
				case Write::Error:
					m_clientFdMap.erase(currEvent->ident);
					close(currEvent->ident);
					break;
				
				case Write::Making:
					break;

				case Write::Complete:
					close(currEvent->ident);
					m_fileFdMap.erase(currEvent->ident);
					break;
				}
			}
		}
	}
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
	tmpInfo.isCgi = false;
	m_clientFdMap.insert(std::pair<int, InfoClient>(clientSocket, tmpInfo));
	m_clientFdMap[clientSocket].m_responserPtr->m_infoClientPtr = &m_clientFdMap[clientSocket];
	m_clientFdMap[clientSocket].status = Res::None;
}
