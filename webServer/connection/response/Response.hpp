#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../../includes/Define.hpp"
#include "../../includes/libraries.hpp"
#include "../../InfoFd.hpp"
#include "ResponseInfo.hpp"
#include "CGI.hpp"

class InfoClient;
class CGI;

class Response : public ResponseInfo
{
	public:
		struct FileEvent
		{
			int fd;
			std::size_t size;
			std::string buffer;
			std::size_t m_totalBytes;
			std::size_t m_sentBytes;
			std::size_t m_pipe_sentBytes;

			FileEvent() : fd(-1), size(0), buffer(""), m_totalBytes(0), m_sentBytes(0), m_pipe_sentBytes(0) {}
		};
		
	public:
		InfoClient *p_infoClient;
		FileEvent m_file;

	public:
		int fds[2];
		bool isCgiIng;
		std::string m_resMsg;

	public:
		int openResponse();
		void startResponse();
		void initResponse();
		int  isValidTarget(std::string &target);

	public:
		int GetCase(std::string &target);
		int PostCase(std::string &target);
		// int DeleteCase(std::string &target);

	public:
		int readFile(int fd);
		int writePipe(int fd);
		int writeClient(int clientSocket);
		size_t changeWritePosition(int n);
		size_t getResMsgSize();
		const char * getWriteResult();
	
	/* clear */
	public:
		void clearFileEvent();

};

#endif
