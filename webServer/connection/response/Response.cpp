#include "Response.hpp"

void
Response::openResponse()
{
	std::string cwdPath = this->getCwdPath();
	std::string srcPath = "";
	if (m_infoClientPtr->reqParser.t_result.method == GET)
	{
		std::cerr << "GET RESPONSE\n";
		/*not working...why .. ?*/
		// if (m_infoClientPtr->reqParser.t_result.target == "/")
		// {
		// 	int fd = open((cwdPath + "/www/statics/index.html").c_str(), O_RDONLY);
		// 	char buff[1024] = {0,};
		// 	read(fd, buff, sizeof(buff));
		// 	write(m_infoClientPtr->m_socketFd, buff, sizeof(buff));
		// 	return ;
		// }
		bool isFile = m_fileManagerPtr->isValidStaticSrc(&m_infoClientPtr->reqParser.t_result.target);
		std::cerr << "isFile :" << isFile << "\n";
		if (isFile == true)
		{
			srcPath = cwdPath + "/www/statics" + m_infoClientPtr->reqParser.t_result.target;
			m_infoClientPtr->status = ResMaking;
			int fd = -1;
			struct stat ss;
			if (stat(srcPath.c_str(), &ss) == -1 || S_ISREG(ss.st_mode) != true ||
				(fd = open(srcPath.c_str(), O_RDONLY)) == -1)
				std::cout << "errorPath failier" << std::endl;
			else
			{
				std::cout << "file size = " << ss.st_size << std::endl;
				std::cout << "fd = " << fd << std::endl;
				fcntl(fd, F_SETFL, O_NONBLOCK);
				m_fileManagerPtr->m_file.fd = fd;
				m_fileManagerPtr->m_infoFileptr = new InfoFile(); // to be deleted
				m_fileManagerPtr->m_infoFileptr->m_infoClientPtr = m_infoClientPtr;
				m_fileManagerPtr->m_infoFileptr->srcPath = srcPath;
			}
		}
		if (isFile == false)
		{
			std::cerr << "	NO FILE FOUND\n";
			//404 response
		}
	}
}

void
Response::initResponse()
{
	setStatusCode(m_infoClientPtr->reqParser.t_result.status);
	setStatusMsg(_statusMap[getStatusCode()]);
	setDate();
	if (m_infoClientPtr->reqParser.t_result.close == true)
		setConnection("close");
	else
		setConnection("keep-alive");
	setContentType("text/html");
	setTransferEncoding("identity");
	setContentLength(m_fileManagerPtr->m_file.buffer.size());
	setBody(m_fileManagerPtr->m_file.buffer);
}

void
Response::startResponse()
{
	initResponse();
	m_resMsg += getHttpVersion() + " " + std::to_string(getStatusCode()) + CRLF;
	m_resMsg += "Connection : " + getConnection() + CRLF;
	m_resMsg += "Date : " + getDate() + CRLF;
	m_resMsg += "Server : " + getServer() + CRLF;
	m_resMsg += "Content-type : " + getContentType() + CRLF;
	m_resMsg += "Transfer-Encoding : " + getTransferEncoding() + CRLF;
	m_resMsg += "Content-Length : " + std::to_string(getContentLength()) + CRLF;
	m_resMsg += "\n";
	m_resMsg += m_fileManagerPtr->m_file.buffer;
	m_totalBytes = m_resMsg.size();
}

const char *
Response::getSendResult() const
{
	return (this->m_resMsg.c_str() + this->m_sentBytes);
}

size_t
Response::getSendResultSize() const
{
	return (this->m_totalBytes - this->m_sentBytes);
}

size_t
Response::changePosition(int n)
{
	if (n > 0)
	{
		if (m_sentBytes + n >= m_totalBytes)
			m_sentBytes = m_totalBytes;
		else
			m_sentBytes += n;
	}
	return (getSendResultSize());
}

int
Response::sendResponse()
{
	size_t n = send(6, getSendResult(), getSendResultSize(), 0);

	if (n < 0)
		return SendError;
	else if (changePosition(n) != 0)
		return SendMaking;
	else
		return SendComplete;
}

void
Response::clearResponseByte()
{
	m_resMsg.clear();
	m_sentBytes = 0;
	m_totalBytes = 0;
}
