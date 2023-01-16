/***************************************************/
/* CODED BY JIN H. BANG ===========================*/
/***************************************************/

#include "Connection.hpp"

class Response;

Connection::Connection()
{}

Connection::~Connection()
{}

void
Connection::connectionLoop()
{
	_eventManager.declareKqueue();


	std::cout << "	::map size : " << _serverMap.size() << "\n" << "	::serverSockets: " << _serverSockets.size() << "\n";

	std::vector<int>::iterator it;
	for (it = _serverSockets.begin(); it != _serverSockets.end(); ++it){
		_eventManager.enrollEventToChangeList(_serverMap[*it]._serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		std::cout << "	enrolled serv socket : " << *it << "\n";
	}

	int eventsNum = 0;
	struct kevent const *currEvent = nullptr;
	while (true)
	{
		eventsNum = _eventManager.senseEvents();
		_eventManager.clearChangeList();
		for (int i = 0; i < eventsNum; ++i)
		{

			currEvent = const_cast<struct kevent const *>(&(_eventManager.getEventList()[i]));

			/* error case */
			if (currEvent->flags & EV_ERROR) {
				if (_serverMap.find(currEvent->ident) != _serverMap.end()) {
					// close(currEvent->ident);
					std::cerr << " server error case \n";
					throw ConnectionError();
				}
				else if (_clientMap.find(currEvent->ident) != _clientMap.end())
				{
					std::cerr << " client error case \n";
					close(currEvent->ident);
					_clientMap.erase(currEvent->ident);
					throw ConnectionError();
				}
			}

			/* read event */
			else if (currEvent->filter == EVFILT_READ)
			{
				if (_serverMap.find(currEvent->ident) != _serverMap.end())
				{
					int clientSocket = accept(currEvent->ident, (sockaddr *)&_serverMap[currEvent->ident]._serverAddr, &_serverMap[currEvent->ident]._serverAddrLen);
					// std::cout << "	accepted client socket : " << clientSocket << "\n"; // test code
					if (clientSocket == FAIL){
						std::cerr << " Error : accept() \n";
						throw ConnectionError();}

					if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == FAIL){
						std::cerr << " Error : fcntl() \n";
						throw ConnectionError();}

					_eventManager.enrollEventToChangeList(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

					_serverMap[currEvent->ident]._clients.push_back(clientSocket);
					InfoClient infoClient; // need to be initialized
					infoClient._clientSocket = clientSocket;
					infoClient._server = &_serverMap[currEvent->ident];

					std::cout << "\n\nINSERT CLIENT : " << clientSocket << "\n";
					std::cout << "SERVER : " << currEvent->ident << "\n\n";

					_clientMap.insert(std::pair<int, InfoClient>(clientSocket, infoClient));
					_clientMap[clientSocket].reqMsg = "";

					Response responser;
					_responserMap.insert(std::pair<int, Response>(clientSocket, responser));

				}

				else if (_clientMap.find(currEvent->ident) != _clientMap.end()) {
					// char buffer[BUFFER_SIZE] = {0};
					char buffer[1024] = {0};
					// std::cout << "	clientMap size : " << _clientMap.size() << "\n";

					int valRead = read(currEvent->ident, buffer, sizeof(buffer));
					// std::cout << "test :: " << buffer << "\n\n";
					if (valRead == FAIL)
					{
						std::cerr << " from client " << currEvent->ident;
						std::cerr << " Error : read() \n";
						// send error page
						close(currEvent->ident);
						_clientMap.erase(currEvent->ident);
					}
					else if (valRead > 0)
					{
						buffer[valRead] = '\0';
						_clientMap[currEvent->ident].req.parseMessage(buffer);
						if (_clientMap[currEvent->ident].req.t_result.pStatus != Request::pComplete && _clientMap[currEvent->ident].req.t_result.pStatus != Request::pError) {
							memset(buffer, 0, sizeof(buffer));
						}
						else if (_clientMap[currEvent->ident].req.t_result.pStatus == Request::pComplete)
						{
							std::cout << "\n\n\nREQUEST------\n";
							std::cout << _clientMap[currEvent->ident].req.t_result.orig << "\n\n";
							std::cout << "\n\n\nBODY-------\n";
							std::cout << _clientMap[currEvent->ident].req.t_result.body << "\n";
							std::cout << "\n\n\nPARSE-------\n";
							_clientMap[currEvent->ident].req.printRequest();
							std::cout << "\n\n\n";
							_eventManager.enrollEventToChangeList(currEvent->ident, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
					}
				}
			}

			/* write event */
			else if (currEvent->filter == EVFILT_WRITE)
			{
				std::map<int, InfoClient>::iterator it = _clientMap.find(currEvent->ident);
				if (it != _clientMap.end()) {
					_responserMap[currEvent->ident].responseToClient(currEvent->ident, _clientMap[currEvent->ident]);
					close(currEvent->ident);
					_clientMap.erase(currEvent->ident);
				}
			}
		}
	}
}
