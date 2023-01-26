#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../includes/libraries.hpp"
#include "../includes/Define.hpp"

#include "../server/Server.hpp"
#include "ResponseInfo.hpp"
#include "request/Request.hpp"
#include "CGI.hpp"

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

	public:
		void openResponse();
		void initResponse();
		void startResponse();

	public:
		int sendResponse();
		size_t changePosition(int n);
		size_t getSendResultSize() const;
		const char * getSendResult() const;
		void clearResponseByte();


	public:
		Client()
		: m_clientFd(-1), ptr_server(NULL), status(0) {}



	/* File */
	public:
		struct FileEvent
		{
			int fd;
			std::size_t size;
			std::string buffer;
			std::size_t m_totalBytes;
			std::size_t m_sentBytes;
			std::size_t m_pipe_sentBytes;
			int inFds[2];
			int outFds[2];
			int isFile;
			std::string srcPath;
			FileEvent() : fd(-1), size(0), buffer(""), m_totalBytes(0), m_sentBytes(0), m_pipe_sentBytes(0){}
		};

	public:
		FileEvent m_file;
		CGI m_cgi;

	public:
		int isValidTarget(std::string &targetPath);
		int	readFile(int fd);
		void clearFileEvent();

	public:
		int writePipe(int fd);

	public:
		std::string getCwdPath()
		{
			char cwd[1024] = {0,};
			getcwd(cwd, 1024);
			std::string cwdPath(cwd);
			return (cwdPath);
		}
};

#endif
