#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "../includes/Define.hpp"
#include "../includes/libraries.hpp"
#include "../server/Server.hpp"
#include "../client/Client.hpp"
#include "../multiplex/Multiplex.hpp"

class Multiplex;

class Log
{
public:
	std::string time;
	std::string method;
	std::string target;
};

class Connection : public Multiplex
{
public:
	std::map<int, Server> m_serverMap;
	std::map<int, Client> m_clientMap;
	std::map<int, Client *> m_fileMap;
	std::map<std::string, std::vector<Log> > m_sessionMap;
	int sizeBuf;

private:
	int eventNum;

public:
	Connection()
		: eventNum(0)
	{
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
	void readyToResponse();
	std::string getMethodToStr(int num);

public:
	void acceptClient();
	void clientReadEvent();
	void fileReadRvent();

	// public:
	// 	void clientWriteEvent();
	// 	void fileWriteEvent();

	// void logStampFromReq();

	std::string idFromReq(Request req)
	{
		std::string cookie = req.t_result.header["Cookie"];
		std::vector<std::string> res = split(cookie, '=');
		return (res[1]);
	}

	Log logFromReq(Request req)
	{
		Log log;

		log.time = logtimeStamp();
		log.method = changeMETHODsToString(req.t_result.method);
		log.target = req.t_result.target;

		return (log);
	}

	std::string logtimeStamp()
	{
		std::string days[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sum"};
		std::string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
		std::time_t timeBuff = std::time(NULL);
		struct tm *time = std::localtime(&timeBuff);
		std::stringstream ss;
		ss << days[time->tm_wday] << ", " << time->tm_mday << " " << months[time->tm_mon] << " " << 1900 + time->tm_year
		   << " ";
		if (time->tm_hour < 10)
			ss << "0";
		ss << time->tm_hour << ":";
		if (time->tm_min < 10)
			ss << "0";
		ss << time->tm_min << ":";
		if (time->tm_sec < 10)
			ss << "0";
		ss << time->tm_sec << " GMT";
		return (ss.str());
	}

	std::vector<std::string> split(std::string input, char delimiter)
	{
		std::vector<std::string> words;
		std::stringstream ss(input);
		std::string temp;
		while (std::getline(ss, temp, delimiter))
		{
			words.push_back(temp);
		}
		return (words);
	}

	std::string changeMETHODsToString(int METHOD)
	{
		std::string result = "";
		switch (METHOD)
		{
		case GET:
			result = "GET";
			break;

		case POST:
			result = "POST";
			break;

		case DELETE:
			result = "DELETE";
			break;
		}
		return (result);
	}

	void logPrint(std::string sessionId, std::vector<Log> logs)
	{
		std::vector<Log>::iterator it;

		for (it = logs.begin(); it != logs.end(); ++it)
		{
			std::cout << "SESSION LOG : "<< sessionId << " requested " << it->method << it->target << " at " << it->time << "\n";
		}
	}
};

#endif
