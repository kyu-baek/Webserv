#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../includes/libraries.hpp"
#include "../includes/Define.hpp"

#include "../server/Server.hpp"
#include "ResponseInfo.hpp"
#include "request/Request.hpp"
#include "File.hpp"

class Client : public ResponseInfo
{
	public:
		int m_clientFd;
		Server *ptr_server;
		Request reqParser;
		bool isCgi;
		std::string m_resMsg;
		size_t m_totalBytes;
		size_t m_sentBytes;
		std::string cgiOutPath;
		std::string cgiOutTarget;

	public:
		int status;
		std::string path;

	public:
		FileEvent m_file;

	public:
		void openResponse();
		void openErrorResponse(int errorCode);
		void initHeader();
		void initResponse();
		void makeResult();
		void startResponse();
		void starAutoindex();
		void openfile(std::string targetPath);
		std::string getExecvePath();

	public:
		int sendResponse();
		size_t changePosition(int n);
		size_t getSendResultSize() const;
		const char * getSendResult() const;
		void clearResponseByte();
		char **initEnv(void);
		std::string cgiFinder(std::string target);
		int isValidTarget(std::string &target);
		int openDirectory(std::string &target);

	public:
		int writePipe(int fd);
		int	readFile(int fd);
		void clearFileEvent();


	public:
		Client()
		: m_clientFd(-1), ptr_server(NULL), status(0), path("") {}


	public:
		std::map<std::string, std::string> initMimeMap();
};

#endif
