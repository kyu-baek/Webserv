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
	if (m_clientMap.find(currEvent->ident) != m_clientMap.end())
	{
		deleteClient(currEvent->ident);
	}

}

void
Connection::handleTimeOut()
{
	std::cout << "\n\n EVFILT_TIMER : " << currEvent->ident << "\n";
	if (m_clientMap.find(currEvent->ident) != m_clientMap.end())
	{
		deleteClient(currEvent->ident);
	}
	std::cout << "\n\n TIMER EVENT DONE-------------------------------------\n";
}

void
Connection::deleteClient(int socket)
{
	std::cout << "DeleteClient : " << socket<<std::endl;
	if (m_clientMap.find(socket) == m_clientMap.end())
		return ;
	std::map <int, Client*>::iterator it;
	for (it = m_fileMap.begin(); it != m_fileMap.end(); it++)
	{
		if (it->second->m_clientFd == (int)socket)
		{
			m_fileMap.erase(it->first);
			continue;
		}
	}
	int server = m_clientMap.find(socket)->second.ptr_server->m_serverFd;
	std::vector<int>::iterator it2;
	for (it2 = m_serverMap.find(server)->second.m_clientVec.begin(); it2 != m_serverMap.find(server)->second.m_clientVec.end(); it2++)
	{
		if (*it2 == socket)
		{

			m_serverMap.find(server)->second.m_clientVec.erase(it2);
			break;
		}
	}
	m_clientMap.erase(socket);
	enrollEventToChangeList(socket, EVFILT_TIMER, EV_DELETE | EV_DISABLE, 0, 0, NULL);
	close(socket);
}

void
Connection::handleReadEvent()
{
	/* Server Event Case */
	if (m_serverMap.find(currEvent->ident) != m_serverMap.end())
	{
		std::cout << "SERVER : " << currEvent->ident << "	=>";
		int clientSocket = accept(currEvent->ident,
									(sockaddr *)&m_serverMap[currEvent->ident].m_serverAddr,
									&m_serverMap[currEvent->ident].m_serverAddrLen);
		if (clientSocket == FAIL)
			std::cerr << "  ERROR : accept() in Server Event Case\n";
		std::cout << "	ACCEPT : " << clientSocket << std::endl;
		setNonBlock(clientSocket);
		enrollEventToChangeList(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		enrollEventToChangeList(clientSocket, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS, TIMER, NULL);
		initClient(clientSocket);
	}

	/* Client Event Case */
	if (m_clientMap.find(currEvent->ident) != m_clientMap.end())
	{
		std::cout << "\n--IN CLIENT : " << currEvent->ident << "\n";
		//system("netstat -an | grep 8080");

		// char buffer[BUFFER_SIZE + 1] = {0, };

		std::vector<char> reqBuffer(BUFFER_SIZE);
		int valRead = recv(currEvent->ident, reqBuffer.data(), reqBuffer.size(), 0);

		std::cout << "valRead :" << valRead << std::endl;
		// std::cout << "RECEIVED BUFFER => \n" << std::string(reqBuffer.begin(), reqBuffer.begin() + valRead) << "\n" << std::endl;
		std::stringstream ss;
		ss << std::string(reqBuffer.begin(), reqBuffer.begin() + valRead);


		if (valRead == FAIL)
		{
			std::cerr << currEvent->ident<<"	ERROR : read() in Client Event Case\n";
			std::vector<int>::iterator it;
			for (it = m_serverMap[m_clientMap[currEvent->ident].ptr_server->m_serverFd].m_clientVec.begin();
					it != m_serverMap[m_clientMap[currEvent->ident].ptr_server->m_serverFd].m_clientVec.end(); ++it)
			{
				if (*it == (int)currEvent->ident)
					break;
			}
			if (it != m_serverMap[m_clientMap[currEvent->ident].ptr_server->m_serverFd].m_clientVec.end())
			{
				m_serverMap[m_clientMap[currEvent->ident].ptr_server->m_serverFd].m_clientVec.at(currEvent->ident);
			}
			close(currEvent->ident);
			m_clientMap.erase(currEvent->ident);
		}
		else if (valRead > 0)
		{
			// std::cout << "\n\nFOR REQEUST ----\n";
			// std::cout << ss.str() << "\n\n";
			// buffer[valRead] = '\0';
			m_clientMap[currEvent->ident].reqParser.makeRequest(ss.str());
			m_clientMap[currEvent->ident].status = Res::None;
			std::cout << "\nREQUEST STATUS => " << m_clientMap[currEvent->ident].reqParser.t_result.pStatus << "\n\n";
			if (m_clientMap[currEvent->ident].reqParser.t_result.pStatus == Request::ParseComplete)
			{
				std::cout << "\n RESULT HEADER \n";
				std::map<std::string, std::string>::iterator it;
				for (it = m_clientMap[currEvent->ident].reqParser.t_result.header.begin(); it != m_clientMap[currEvent->ident].reqParser.t_result.header.end(); ++it)
				{
					std::cout << "=>" << it->first << " = " << it->second <<"\n";

				}

				std::cout << "\n --REQUEST FROM CLIENT " << currEvent->ident << "--\n :: "
							  << m_clientMap[currEvent->ident].reqParser.t_result.orig << "\n\n";
					m_clientMap[currEvent->ident].reqParser.t_result.orig = "";

				if (m_clientMap[currEvent->ident].status == Res::None)
				{
					m_clientMap[currEvent->ident].openResponse();
					if (m_clientMap[currEvent->ident].isCgi == false)
					{
						if (m_clientMap[currEvent->ident].m_file.fd != -1)
						{
							int fileFd = m_clientMap[currEvent->ident].m_file.fd;
							std::cout << "m_file.fd : " << fileFd << std::endl;
							enrollEventToChangeList(fileFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							fcntl(fileFd, F_SETFL, O_NONBLOCK);
							m_fileMap.insert(std::make_pair(fileFd, &m_clientMap[currEvent->ident]));
						}
						else
							std::cerr << "ERROR : openResponse() " << m_clientMap[currEvent->ident].getStatusCode() << "\n";
					}
					else
					{
						std::cout << "	cgi true" << std::endl;
						std::cout << "inFds[0] : " << m_clientMap[currEvent->ident].m_file.inFds[0] << "inFds[1]" << m_clientMap[currEvent->ident].m_file.inFds[1] <<std::endl;
						std::cout << "outFds[0] : " << m_clientMap[currEvent->ident].m_file.outFds[0] << "outFds[1]" << m_clientMap[currEvent->ident].m_file.outFds[1] <<std::endl;

						int pipeWrite = m_clientMap[currEvent->ident].m_file.inFds[1];
						int pipeRead = m_clientMap[currEvent->ident].m_file.outFds[0];

						enrollEventToChangeList(pipeWrite, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						fcntl(pipeWrite, F_SETFL, O_NONBLOCK);
						m_fileMap.insert(std::make_pair(pipeWrite, &m_clientMap[currEvent->ident]));

						// enrollEventToChangeList(pipeRead, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						// fcntl(pipeRead, F_SETFL, O_NONBLOCK);
						m_fileMap.insert(std::make_pair(pipeRead, &m_clientMap[currEvent->ident]));
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
			if (m_clientMap[currEvent->ident].reqParser.t_result.pStatus == Request::ParseError)
			{
				std::cerr << "	Error : parse\n";
				// 404
			}
		}
	}

	/* File Event Case */
	if (m_fileMap.find(currEvent->ident) != m_fileMap.end())
	{

		std::cout << "FILE READ : " << currEvent->ident << std::endl;
		if (m_fileMap[currEvent->ident]->status == Res::Making)
		{

			int res = m_fileMap[currEvent->ident]->readFile(currEvent->ident);

			switch (res)
			{
			case File::Error:
				// 500 error page open
				close(currEvent->ident);
				m_fileMap.erase(currEvent->ident);
				m_fileMap[currEvent->ident]->m_file.buffer.clear();
				std::cout << "fError" << std::endl;
				break;
			case File::Making:
				// std::cout << "fMaking = " << std::endl;
				break;
			case File::Complete:
				std::cout << "Complete" << std::endl;
				//enrollEventToChangeList(currEvent->ident, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
				// std::cout << "\n\nIN m_file.buffer => \n" << m_fileMap[currEvent->ident]->m_file.buffer << "\n\n";
				m_fileMap[currEvent->ident]->status = Res::Complete;
				m_fileMap[currEvent->ident]->startResponse();
				std::cout << "\n\n++++++ FILE READ COMPLETE : m_resMsg => \n" << m_fileMap[currEvent->ident]->m_resMsg << "\n\n";
				enrollEventToChangeList(m_fileMap[currEvent->ident]->m_clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				close(currEvent->ident);
				m_fileMap.erase(m_fileMap.find(currEvent->ident)->first);

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

		if (m_clientMap.find(currEvent->ident) != m_clientMap.end())
		{
			std::cout << "CLIENT WRITE : " << currEvent->ident << std::endl;
			int result;
			// if (m_clientMap[currEvent->ident].isCgi == false)
			result = m_clientMap[currEvent->ident].sendResponse();
			// if (m_clientMap[currEvent->ident].isCgi == true)
			// {
			// 	result = m_clientMap[currEvent->ident].m_responserPtr->sendResponse();
			// }

			switch (result)
			{
			case Send::Error:
				// 500 error page open
				// std::cout << "fError" << std::endl;
				m_clientMap[currEvent->ident].status = Res::None;
				break;
			case Send::Making:
				// keep reading
				m_clientMap[currEvent->ident].status = Res::Making;
				break;
			case Send::Complete:
					std::cout << "	--RESPONSE SENT TO CLIENT " << currEvent->ident << "--\n\n";

					enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, NULL);
					m_clientMap[currEvent->ident].clearResInfo();
					m_clientMap[currEvent->ident].clearResponseByte();
					m_clientMap[currEvent->ident].clearFileEvent();
					m_clientMap[currEvent->ident].status = Res::Complete;
					m_clientMap[currEvent->ident].reqParser.clearRequest();
					if (m_clientMap[currEvent->ident].isCgi == true)
					{
						m_clientMap[currEvent->ident].isCgi = false;
						m_clientMap[currEvent->ident].cgiOutPath = "";
						m_clientMap[currEvent->ident].cgiOutTarget = "";
					}
					waitpid(m_clientMap[currEvent->ident].m_file.pid, NULL, WNOHANG);
				break;
			}

		}
		if ((m_fileMap.find(currEvent->ident) != m_fileMap.end()) && (m_fileMap[currEvent->ident]->isCgi == true))
		{
			std::cout << "FILE WRITE : " << currEvent->ident << std::endl;
			int result = m_fileMap[currEvent->ident]->writePipe(currEvent->ident);
			std::cout << "		RESULT OF CGI WRITE : " << result << "\n\n";
			switch (result)
			{
			case Write::Error:
				m_clientMap.erase(currEvent->ident);
				//자원정리
				close(currEvent->ident);
				break;

			case Write::Making:
				break;

			case Write::Complete:
				//자원정리
				enrollEventToChangeList(m_fileMap[currEvent->ident]->m_file.outFds[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
				fcntl(m_fileMap[currEvent->ident]->m_file.outFds[0], F_SETFL, O_NONBLOCK);

				enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, NULL);
				close(currEvent->ident);
				m_fileMap.erase(currEvent->ident);
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
	if (m_serverMap.find(currEvent->ident) != m_serverMap.end())
	{
		std::cout << "1111111\n";
		if (m_clientMap.empty() == true)
			return ;
		this->m_serverMap.erase(this->m_serverMap.find(currEvent->ident));
		close(currEvent->ident);
	}
	else if (this->m_clientMap.find(currEvent->ident) != this->m_clientMap.end())
	{
		std::cout << "2222\n";
		//m_fileMap.find()
		this->m_clientMap.erase(this->m_clientMap.find(currEvent->ident));
		close(currEvent->ident);
		//map 도 지우기
	}
	if (this->m_fileMap.find(currEvent->ident) != this->m_fileMap.end())
	{
		std::cout << "3333\n";
		this->m_fileMap.erase(this->m_fileMap.find(currEvent->ident));
		close(currEvent->ident);
	}
	// this->m_fileMap.erase(this->m_fileMap.find(currEvent->ident));
	// close(currEvent->ident);
}

void
Connection::setNonBlock(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == FAIL)
		std::cerr << "	ERROR : fcntl()";
}

void
Connection::initClient(int clientSocket)
{
	m_serverMap[currEvent->ident].m_clientVec.push_back(clientSocket);

	Client tmpClient;
	tmpClient.m_clientFd = clientSocket;
	tmpClient.ptr_server = &m_serverMap[currEvent->ident];
	tmpClient.isCgi = false;
	tmpClient.status = Res::None;
	tmpClient.m_resMsg = "";
	tmpClient.m_totalBytes = 0;
	tmpClient.m_sentBytes = 0;
	tmpClient.cgiOutPath = "";
	tmpClient.cgiOutTarget = "";
	tmpClient.m_file.buffer = "";
	tmpClient.m_file.fd = -1;
	tmpClient.m_file.m_pipe_sentBytes = 0;
	tmpClient.m_file.m_sentBytes = 0;
	tmpClient.m_file.m_totalBytes = 0;
	tmpClient.m_file.size = 0;
	m_clientMap.insert(std::pair<int, Client>(clientSocket, tmpClient));
}
