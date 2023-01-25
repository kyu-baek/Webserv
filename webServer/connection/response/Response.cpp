#include "Response.hpp"

void
Response::openResponse()
{
	std::string cwdPath = this->getCwdPath();
	std::string srcPath = "";
	std::string execPath = "";

	
	this->_statusCode = m_fileManagerPtr->isValidTarget(m_infoClientPtr->reqParser.t_result.target);
	std::cout << "_statusCode : " << _statusCode << std::endl;
	if (this->_statusCode >= 400)
	{
		//send Error msg;
		std::cerr <<m_infoClientPtr->m_socketFd << "	ERROR : INVALID TARGET : " << m_infoClientPtr->reqParser.t_result.target << std::endl;
		return;
	}

	if (m_infoClientPtr->reqParser.t_result.method == GET)
	{
		std::cerr << "GET RESPONSE\n";
		std::cerr << "isFile :" << this->_statusCode << "\n";
		if (this->_statusCode == 200)
		{
			srcPath = cwdPath + "/www/statics" + m_infoClientPtr->reqParser.t_result.target;
			m_infoClientPtr->status = Res::Making;
			int fd = -1;
			struct stat ss;
			std::cout << "srcPath : "<<srcPath << std::endl;
			if (stat(srcPath.c_str(), &ss) == -1 || S_ISREG(ss.st_mode) != true || (fd = open(srcPath.c_str(), O_RDONLY)) == -1)
				this->_statusCode = 500;
			else
			{
				m_fileManagerPtr->m_file.fd = fd;
				m_fileManagerPtr->m_infoFileptr = new InfoFile(); // to be deleted
				m_fileManagerPtr->m_infoFileptr->m_infoClientPtr = m_infoClientPtr;
				m_fileManagerPtr->m_infoFileptr->srcPath = srcPath;
				m_infoClientPtr->status = Res::Making; // added
			}
		}
		if (this->_statusCode == 404 || this->_statusCode == 500)
		{
			std::cerr << "	NO FILE FOUND\n";
			//404 response
		}
	}

	if (m_infoClientPtr->reqParser.t_result.method == POST)
	{
		cwdPath = this->getCwdPath();
		execPath = getCwdPath() + "/www/cgi-bin" + m_infoClientPtr->reqParser.t_result.target;
		cgiOutTarget = "cgiout_" + std::to_string(m_infoClientPtr->m_socketFd) + ".html";
		cgiOutPath = getCwdPath() + "/" + cgiOutTarget;

		// if (pipe(m_fileManagerPtr->m_infoFileptr->inFds) < 0 || pipe(m_fileManagerPtr->m_infoFileptr->outFds) < 0)
		// 	std::cerr <<"ERROR: pipe\n";

	// 	 char* argv2[3] = {
	// 		(char*)execPath.c_str(),
	// 		(char*)NULL,
	// 		0
	// };
	std::string str("/usr/bin/python3");
	char* argv2[3] = {
			(char*)str.c_str(),
			(char*)execPath.c_str(),
			0
	};
		std::cerr << "execPath : " << execPath << std::endl;
		char const *args[3] = {execPath.c_str(), NULL, 0};

		m_fileManagerPtr->m_cgi.initEnvMap();
		if (m_infoClientPtr->reqParser.t_result.method == GET)
			m_fileManagerPtr->m_cgi.envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "GET"));
		else if (m_infoClientPtr->reqParser.t_result.method == POST)
			m_fileManagerPtr->m_cgi.envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "POST"));
		else if (m_infoClientPtr->reqParser.t_result.method == DELETE)
			m_fileManagerPtr->m_cgi.envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "DELETE"));
		m_fileManagerPtr->m_cgi.envMap.insert(std::pair<std::string, std::string>("QUERY_STRING", m_infoClientPtr->reqParser.t_result.query));
		m_fileManagerPtr->m_cgi.envMap.insert(std::pair<std::string, std::string>("SERVER_PORT", std::to_string(m_infoClientPtr->m_server->m_port)));
		m_fileManagerPtr->m_cgi.envMap.insert(std::pair<std::string, std::string>("UPLOAD_PATH", cwdPath + "/db/"));
		m_fileManagerPtr->m_cgi.envMap.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", args[0]));

		char **envs = new char *[sizeof(char *) * m_fileManagerPtr->m_cgi.envMap.size()];
		int i = 0;
		for (std::map<std::string, std::string>::iterator it = m_fileManagerPtr->m_cgi.envMap.begin();
			 it != m_fileManagerPtr->m_cgi.envMap.end(); ++it)
		{
			envs[i] = strdup((it->first + "=" + it->second).c_str());
			++i;
		}

		if (pipe(m_fileManagerPtr->m_infoFileptr->inFds) == -1)
			std::cerr <<"ERROR: pipe\n";
		if (pipe(m_fileManagerPtr->m_infoFileptr->outFds) == -1)
		{
			close(m_fileManagerPtr->m_infoFileptr->inFds[0]);
			close(m_fileManagerPtr->m_infoFileptr->inFds[1]);
			std::cerr <<"ERROR: pipe\n";
		}

		std::cout << "111\n";
		std::cout << "inFds[0] : " << m_fileManagerPtr->m_infoFileptr->inFds[0] << " inFds[1] : " << m_fileManagerPtr->m_infoFileptr->inFds[1] <<std::endl;
		std::cout << "outFds[0] : " << m_fileManagerPtr->m_infoFileptr->outFds[0] << " outFds[1] : " << m_fileManagerPtr->m_infoFileptr->outFds[1] <<std::endl;

		int pid = fork();
		if (pid == -1)
		{
			close(m_fileManagerPtr->m_infoFileptr->inFds[0]);
			close(m_fileManagerPtr->m_infoFileptr->inFds[1]);

			close(m_fileManagerPtr->m_infoFileptr->outFds[0]);
			close(m_fileManagerPtr->m_infoFileptr->outFds[1]);
			std::cerr <<"ERROR: return 500\n";
		}
		if (pid > 0)
		{
			std::cout << "	This is Parent of POST : \n";
			std::cout << "222\n";
			std::cout << "inFds[0] : " << m_fileManagerPtr->m_infoFileptr->inFds[0] << " inFds[1] : " << m_fileManagerPtr->m_infoFileptr->inFds[1] <<std::endl;
			std::cout << "outFds[0] : " << m_fileManagerPtr->m_infoFileptr->outFds[0] << " outFds[1] : " << m_fileManagerPtr->m_infoFileptr->outFds[1] <<std::endl;

			close(m_fileManagerPtr->m_infoFileptr->inFds[0]);
			close(m_fileManagerPtr->m_infoFileptr->outFds[1]);

			// waitpid(pid, NULL, WNOHANG);

			m_infoClientPtr->isCgi = true;

			
			// m_fileManagerPtr->m_file.fd = m_fileManagerPtr->m_infoFileptr->inFds[1];
			// m_fileManagerPtr->m_infoFileptr = new InfoFile(); // to be deleted
			// m_fileManagerPtr->m_infoFileptr->m_infoClientPtr = m_infoClientPtr;
			// m_fileManagerPtr->m_infoFileptr->srcPath = ""; // to be updated when isCgiDone == true
			m_infoClientPtr->status = Res::Making; // added
		
		}
		else if (pid == 0)
		{
			//std::cout << "	This is Child of POST : \n";
		

			close(m_fileManagerPtr->m_infoFileptr->inFds[1]);
			dup2(m_fileManagerPtr->m_infoFileptr->inFds[0], STDIN_FILENO);
			close(m_fileManagerPtr->m_infoFileptr->inFds[0]);
			
			close(m_fileManagerPtr->m_infoFileptr->outFds[0]);
			dup2(m_fileManagerPtr->m_infoFileptr->outFds[1], STDOUT_FILENO);
			close(m_fileManagerPtr->m_infoFileptr->outFds[1]);
			// std::cout << "after dub2 \n";
			// std::cout << "inFds[0] : " << m_fileManagerPtr->m_infoFileptr->inFds[0] << "inFds[1] : " << m_fileManagerPtr->m_infoFileptr->inFds[1] <<std::endl;
			// std::cout << "outFds[0] : " << m_fileManagerPtr->m_infoFileptr->outFds[0] << "outFds[1] : " << m_fileManagerPtr->m_infoFileptr->outFds[1] <<std::endl;
			std::cerr << "execPath : " << execPath << std::endl;

			execve(execPath.c_str(), argv2, envs);
			//std::cout << errno << std::endl;
			std::cerr << " ERRRRRRRRRR EXECVE\n";
			exit(EXIT_SUCCESS);
		}
	}
}

void
Response::initResponse()
{
	setStatusCode(m_infoClientPtr->reqParser.t_result.status);
	setStatusMsg(_statusMap[getStatusCode()]);
	setDate();
	if (m_infoClientPtr->reqParser.t_result.close == false)
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
	size_t n = send(m_infoClientPtr->m_socketFd, getSendResult(), getSendResultSize(), 0);

	if (n < 0)
		return Send::Error;
	else if (changePosition(n) != 0)
		return Send::Making;
	else
		return Send::Complete;
}

void
Response::clearResponseByte()
{
	m_resMsg.clear();
	m_sentBytes = 0;
	m_totalBytes = 0;
}
