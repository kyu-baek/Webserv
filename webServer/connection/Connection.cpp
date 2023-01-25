#include "Connection.hpp"


void
Connection::eventLoop()
{
	while (true)
	{
		eventNum = senseEvents();
		clearChangeList();
		for (int i = 0; i < eventNum; i++)
		{
			currEvent = &getEventList()[i];
			if (currEvent->flags & EV_EOF)
				handleEofEvent();
			else if (currEvent->flags & EV_ERROR)
				handleErrorEvent();
			if (currEvent->filter == EVFILT_TIMER)
				handleTimeOut();
			else if (currEvent->filter == EVFILT_READ)
				handleReadEvent();
			else if (currEvent->filter == EVFILT_WRITE)
				handleWriteEvent();
		}
	}
}

void
Connection::handleEofEvent()
{
	std::cout << "	HandleEofEvent : " << currEvent->ident << std::endl;
	if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
	{
		std::cout << "aaaaaa \n" << std::endl; 
		deleteClient(currEvent->ident);
	}

}

void
Connection::handleTimeOut()
{
	std::cout << "\n\n EVFILT_TIMER : " << currEvent->ident << "\n";
	if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
	{
		deleteClient(currEvent->ident);
	}
	std::cout << "\n\n TIMER EVENT DONE-------------------------------------\n";
}

void
Connection::deleteClient(int socket)
{
	std::cout << "DeleteClient : " << socket<<std::endl;
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
	int server = m_clientFdMap.find(socket)->second.m_server->m_serverSocket;
	std::vector<int>::iterator it2;
	for (it2 = m_serverFdMap.find(server)->second.m_clients.begin(); it2 != m_serverFdMap.find(server)->second.m_clients.end(); it2++)
	{
		if (*it2 == socket)
		{

			m_serverFdMap.find(server)->second.m_clients.erase(it2);
			break;
		}
	}
	delete m_clientFdMap.find(socket)->second.m_responserPtr->m_fileManagerPtr;
	delete m_clientFdMap.find(socket)->second.m_responserPtr;
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
		std::cout << "SERVER READ : " << currEvent->ident << std::endl;
		int clientSocket = accept(currEvent->ident,
									(sockaddr *)&m_serverFdMap[currEvent->ident].m_serverAddr,
									&m_serverFdMap[currEvent->ident].m_serverAddrLen);
		if (clientSocket == FAIL)
			std::cerr << "  ERROR : accept() in Server Event Case\n";
		std::cout << "	ACCEPT : " << clientSocket << std::endl;
		setNonBlock(clientSocket);
		enrollEventToChangeList(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		enrollEventToChangeList(clientSocket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER, NULL);
		initInfoClient(clientSocket);
	}

	/* Client Event Case */
	if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
	{
		std::cout << "CLIENT READ : " << currEvent->ident << std::endl;
		//system("netstat -an | grep 8080");
		char buffer[BUFFER_SIZE + 1] = {0, };
		ssize_t valRead = read(currEvent->ident, buffer, BUFFER_SIZE);
		std::cout << "valRead :" << valRead << std::endl;
		if (valRead == FAIL)
		{
			std::cerr << currEvent->ident<<"	ERROR : read() in Client Event Case\n";
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

			if (m_clientFdMap[currEvent->ident].reqParser.t_result.pStatus == Request::ParseComplete)
			{
				if (m_clientFdMap[currEvent->ident].reqParser.t_result.target == "/favicon.ico")
					return ;
				if (m_clientFdMap[currEvent->ident].status == Res::None)
				{
					m_clientFdMap[currEvent->ident].m_responserPtr->openResponse();
					if (m_clientFdMap[currEvent->ident].isCgi == false)
					{
						if (m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_file.fd != -1)
						{
							int fileFd = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_file.fd;
							std::cout << "m_file.fd : " << fileFd << std::endl;
							enrollEventToChangeList(fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							fcntl(fileFd, F_SETFL, O_NONBLOCK);
							m_fileFdMap.insert(std::make_pair(fileFd, *(m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr)));
							m_fileFdMap[fileFd].m_fileManagerPtr = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr;
							m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->m_fileFdMapPtr = &m_fileFdMap;
						}
						// else
							// std::cerr << "ERROR : openResponse() " << m_clientFdMap[currEvent->ident].m_responserPtr->getStatusCode() << "\n";
					}
					else
					{
						std::cout << "	cgi true" << std::endl;
						std::cout << "inFds[0] : " << m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->inFds[0] << "inFds[1]" << m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->inFds[1] <<std::endl;
						std::cout << "outFds[0] : " << m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->outFds[0] << "outFds[1]" << m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->outFds[1] <<std::endl;

						int pipeWrite = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->inFds[1];
						int pipeRead = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->outFds[0];

						enrollEventToChangeList(pipeWrite, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						fcntl(pipeWrite, F_SETFL, O_NONBLOCK);
						m_fileFdMap.insert(std::make_pair(pipeWrite, *(m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr)));
						m_fileFdMap[pipeWrite].m_fileManagerPtr = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr;
						m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->m_fileFdMapPtr = &m_fileFdMap;

						// enrollEventToChangeList(pipeRead, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						// fcntl(pipeRead, F_SETFL, O_NONBLOCK);
						m_fileFdMap.insert(std::make_pair(pipeRead, *(m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr)));
						m_fileFdMap[pipeRead].m_fileManagerPtr = m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr;
						m_clientFdMap[currEvent->ident].m_responserPtr->m_fileManagerPtr->m_infoFileptr->m_fileFdMapPtr = &m_fileFdMap;
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

		std::cout << "FILE READ : " << currEvent->ident << std::endl;
		if (m_fileFdMap[currEvent->ident].m_infoClientPtr->status == Res::Making)
		{

			int res = m_fileFdMap[currEvent->ident].m_fileManagerPtr->readFile(currEvent->ident);
	
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
				//enrollEventToChangeList(currEvent->ident, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
				m_fileFdMap[currEvent->ident].m_infoClientPtr->status = Res::Complete;
				if (m_fileFdMap[currEvent->ident].isCgi == false)
				{
					std::cout << "start\n\n";
					m_fileFdMap[currEvent->ident].m_infoClientPtr->m_responserPtr->startResponse();
					enrollEventToChangeList(m_fileFdMap[currEvent->ident].m_infoClientPtr->m_socketFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					close(currEvent->ident);
					m_fileFdMap.erase(m_fileFdMap.find(currEvent->ident)->first);
					std::cout << "555\n\n";
				}
				else	// std::cout << "cgi\n";
				{
				
					enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				}

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
		std::cout << "\n\n WRITE EVENT : " << currEvent->ident << std::endl;
		
		if (m_clientFdMap.find(currEvent->ident) != m_clientFdMap.end())
		{
			std::cout << "CLIENT WRITE : " << currEvent->ident << std::endl;
			int result;
			// if (m_clientFdMap[currEvent->ident].isCgi == false)
				result = m_clientFdMap[currEvent->ident].m_responserPtr->sendResponse();
			// if (m_clientFdMap[currEvent->ident].isCgi == true)
			// {
			// 	result = m_clientFdMap[currEvent->ident].m_responserPtr->sendResponse();
			// }

			switch (result)
			{
			case Send::Error:
				// 500 error page open
				// std::cout << "fError" << std::endl;
				m_clientFdMap[currEvent->ident].status = Res::None;
				break;
			case Send::Making:
				// keep reading
				m_clientFdMap[currEvent->ident].status = Res::Making;
				break;
			case Send::Complete:
					std::cout << "	--RESPONSE SENT TO CLIENT " << currEvent->ident << "--\n\n";
			
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
					//enrollEventToChangeList(currEvent->ident, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					// std::cout<< "\n\nresponse msg : \n" << m_clientFdMap[currEvent->ident].m_responserPtr->m_resMsg << std::endl;


				break;
			}

		}
		if ((m_fileFdMap.find(currEvent->ident) != m_fileFdMap.end()) && (m_fileFdMap[currEvent->ident].m_infoClientPtr->isCgi == true))
		{
			std::cout << "FILE WRITE : " << currEvent->ident << std::endl;
			int result = m_fileFdMap[currEvent->ident].m_infoClientPtr->m_responserPtr->m_fileManagerPtr->writePipe(currEvent->ident);
			std::cout << "		RESULT OF CGI WRITE : " << result << "\n\n";
			switch (result)
			{
			case Write::Error:
				m_clientFdMap.erase(currEvent->ident);
				//자원정리
				close(currEvent->ident);
				break;
			
			case Write::Making:
				break;

			case Write::Complete:
				
				//자원정리
				enrollEventToChangeList(m_fileFdMap[currEvent->ident].outFds[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
				fcntl(m_fileFdMap[currEvent->ident].outFds[0], F_SETFL, O_NONBLOCK);
				enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, NULL);
				close(currEvent->ident);
				m_fileFdMap.erase(currEvent->ident);
				std::cout << "close : " << currEvent->ident << std::endl;
				break;
			}
		}
	}
}

void
Connection::handleErrorEvent()
{

	std::cout << "handleErrorEvent : " << currEvent->ident << std::endl;
	if (m_serverFdMap.find(currEvent->ident) != m_serverFdMap.end())
	{
		std::cout << "1111111\n";
		if (m_clientFdMap.empty() == true)
			return ;
		this->m_serverFdMap.erase(this->m_serverFdMap.find(currEvent->ident));
		close(currEvent->ident);
	}
	else if (this->m_clientFdMap.find(currEvent->ident) != this->m_clientFdMap.end())
	{
		std::cout << "2222\n";
		//m_fileFdMap.find()
		this->m_clientFdMap.erase(this->m_clientFdMap.find(currEvent->ident));
		close(currEvent->ident);
		//map 도 지우기
	}
	if (this->m_fileFdMap.find(currEvent->ident) != this->m_fileFdMap.end())
	{
		std::cout << "3333\n";
		this->m_fileFdMap.erase(this->m_fileFdMap.find(currEvent->ident));
		close(currEvent->ident);
	}
	// this->m_fileFdMap.erase(this->m_fileFdMap.find(currEvent->ident));
	// close(currEvent->ident);
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
	//tmpInfo.m_responserPtr = new Response(); //delete needed
	//tmpInfo.m_responserPtr->m_fileManagerPtr = new FileManage(); // delete needed
	tmpInfo.isCgi = false;
	m_clientFdMap.insert(std::pair<int, InfoClient>(clientSocket, tmpInfo));
	m_clientFdMap[clientSocket].m_responserPtr = new Response();
	m_clientFdMap[clientSocket].m_responserPtr->m_fileManagerPtr = new FileManage();
	m_clientFdMap[clientSocket].m_responserPtr->m_infoClientPtr = &(m_clientFdMap[clientSocket]);
	m_clientFdMap[clientSocket].status = Res::None;
}
