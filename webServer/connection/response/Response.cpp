#include "Response.hpp"

int
Response::openResponse()
{
	std::string cwdPath = this->getCwdPath();
	std::string srcPath = "";
	std::string target = p_infoClient->reqParser.t_result.target;

	int isFile = isValidTarget(target);
	// int status = 0;
	int fd = -1;
	std::cout << "	isFile = " << isFile << "\n\n";
	if (isFile >= 400)
	{
		//error case
		// int fd = open("error.html", O_RDONLY);
		// return (fd);
	}

	int method = p_infoClient->reqParser.t_result.method;
	switch (method)
	{
	case GET:
		fd = this->GetCase(target);
		if (fd == -1)
			std::cout << "fd == -1\n";
			// fd = open("500error.html", O_RDONLY);
		break;
	case POST:
		if (isCgiIng != true)
			this->PostCase(target);
		break;

	// case DELETE:
	// 	this->DeleteCase(target);
	// 	break;
	}
	return (fd);
}
void
Response::initResponse()
{
	setStatusCode(p_infoClient->reqParser.t_result.status);
	setStatusMsg(_statusMap[getStatusCode()]);
	setDate();
	if (p_infoClient->reqParser.t_result.close == true)
		setConnection("close");
	else
		setConnection("keep-alive");
	setContentType("text/html");
	setTransferEncoding("identity");
	setContentLength(m_file.buffer.size());
	setBody(m_file.buffer);
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
	m_resMsg += getResponseBody();
	m_file.m_totalBytes = m_resMsg.size();
}

/*
	check 사항
	1. 읽기 권한이 있는지
	2. 현재경로에서 실행가능 한한지지
	3. client max body size 확인
*/
int
Response::isValidTarget(std::string &target)
{
	std::map<std::string, Location >::iterator it = p_infoClient->m_server->m_location.find(target);
	if (it != p_infoClient->m_server->m_location.end())
	{
		std::string staticPath;
		if (it->second.root != "")
			staticPath = this->getCwdPath() + "/" + it->second.root;
		else
			staticPath = this->getCwdPath()  + "/";
		
		if (it->second.index[0] != "")
			target = it->second.index[0];
		std::cout << "target : " << target << std::endl;
		DIR *dir;
		if ((dir = opendir(staticPath.c_str())) == NULL)
		{
			if (errno == ENOTDIR)
				return 0;
			else if (errno == EACCES)
				return 403;
			else if (errno == ENOENT)
				return 404;
			else if (errno == EMFILE)
				return 503;
			else
				return 500;
		}
		struct dirent *dirent = NULL;
		while (true)
		{
			dirent = readdir(dir);
			if (!dirent)
				break;
			if (strcmp(dirent->d_name, (target).c_str()) == SUCCESS)
			{
				(target).insert(0, "/");
				struct stat ss;
				std::string resPath = staticPath + target;
				if (stat(resPath.c_str(), &ss) == -1 || S_ISREG(ss.st_mode) != true)
					return (500);
				std::cout << "resPath : " << resPath << std::endl;
				return (200);
			}
		}
	}
	return (404);
}


int
Response::GetCase(std::string &target)
{
	std::cout << "getCase\n";
	std::string srcPath = getCwdPath() + "/www/statics" + target;
	// p_infoClient->status = Res::Making;
	int fd = -1;

	std::cout << "srcPath : " << srcPath << std::endl;
	if ((fd = open(srcPath.c_str(), O_RDONLY, 0644)) == -1)
		return (fd);
	return (fd);
}

int
Response::PostCase(std::string &target)
{
	std::cout << "PostCase\n";
	std::string cwdPath = this->getCwdPath();
	std::string execPath = getCwdPath() + "/www/cgi-bin" + target;
	char const *args[2] = {execPath.c_str(), NULL};

	CGI cgi;
	cgi.initEnvMap();
	if (p_infoClient->reqParser.t_result.method == GET)
		cgi.envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "GET"));
	else if (p_infoClient->reqParser.t_result.method == POST)
		cgi.envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "POST"));
	else if (p_infoClient->reqParser.t_result.method == DELETE)
		cgi.envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "DELETE"));
	cgi.envMap.insert(std::pair<std::string, std::string>("QUERY_STRING", p_infoClient->reqParser.t_result.query));
	cgi.envMap.insert(std::pair<std::string, std::string>("SERVER_PORT", std::to_string(p_infoClient->m_server->m_port)));
	cgi.envMap.insert(std::pair<std::string, std::string>("UPLOAD_PATH", cwdPath + "/uploaded/"));
	cgi.envMap.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", args[0]));

	// char *cgiEnv[cgi.envMap.size() + 1];
	char **cgiEnv = new char *[sizeof(char *) *cgi.envMap.size() ];
	cgiEnv[cgi.envMap.size()] = NULL;
	int i = 0;
	for (std::map<std::string, std::string>::iterator iter = cgi.envMap.begin(); iter != cgi.envMap.end(); ++iter)
	{
		cgiEnv[i] = strdup((iter->first + "=" + iter->second).c_str());
		i++;
	}

	pipe(fds);
	int pid = fork();
	if (pid != CHILD)
	{
		// char buff[BUFFER_SIZE] = {0,};
		// std::string partStr = p_infoClient->reqParser.t_result.body.substr(pos, BUFFER_SIZE);
		// pos = 0;
		// pos += BUFFER_SIZE;
		// strcpy(buff, partStr.c_str());

		close(fds[0]);
		 waitpid(pid, &status, WNOHANG);
		// if (res == pid && WEXITSTATUS(status) != 0)
		// {
		// 	status = 
		// }
		p_infoClient->m_responser->isCgiIng = CGIFL::Making;
		return (fds[1]);


		// write(fds[1], buff, sizeof(buff));
		// // std::cerr << "		test\n" << buff << "\n\n";
		// close(fds[1]);

		// waitpid(pid, NULL, 0);

		// /* msg to client */
		// resMsg = resMsgHeader(infoClient);
		// resMsg += "\n" + resMsgBody(filePath);
		// unlink(filePath.c_str()); // if you want to check output html, delete it
	}
	else
	{
		close(fds[1]);
		dup2(fds[0], STDIN_FILENO);
		close(fds[0]);
		std::string outputPath = getCwdPath() + "/cgiout_" +  std::to_string(p_infoClient->m_socketFd) + ".html";
		int resFd = open(outputPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0744);
		if (resFd < 0)
			std::cerr << "Error : resFd open()\n";
		dup2(resFd, STDOUT_FILENO);
		close(resFd);
		execve(execPath.c_str(), const_cast<char* const*>(args), cgiEnv);
		exit(EXIT_SUCCESS);
	}
}

// int
// Response::DeleteCase(std::string &target)
// {

// }


int
Response::readFile(int fd)
{
	char buffer[BUFFER_SIZE + 1];

	memset(buffer, 0, sizeof(buffer));
	//std::cout << "reading\n";
	ssize_t size = read(fd, buffer, sizeof(buffer));
	if (size < 0)
	{
		close(fd);
		//m_infoFileptr->m_fileFdMapPtr->erase(fd);
		m_file.buffer.clear();
		return File::Error;
	}
	m_file.buffer += std::string(buffer, size);
	m_file.size += size;
	if (size < BUFFER_SIZE)
	{
		return File::Complete;
	}
	return File::Making;
}

int
Response::writeClient(int clientSocket)
{
	size_t n = send(clientSocket, getWriteResult(), getResMsgSize(), 0);

	if (n < 0)
		return Send::Error;
	else if (changeWritePosition(n) != 0)
		return Send::Making;
	else
		return Send::Complete;
}

size_t
Response::getResMsgSize()
{
	return (this->m_file.m_totalBytes - this->m_file.m_sentBytes);
}

const char *
Response::getWriteResult()
{
	return (this->m_resMsg.c_str() + this->m_file.m_sentBytes);
}

size_t
Response::changeWritePosition(int n)
{
	if (n > 0)
	{
		if (m_file.m_sentBytes + n >= m_file.m_totalBytes)
			m_file.m_sentBytes = m_file.m_totalBytes;
		else
			m_file.m_sentBytes += n;
	}
	return (getResMsgSize());
}

int
Response::writePipe(int fd)
{
	size_t size;

	size = write(fd, p_infoClient->reqParser.t_result.body.c_str() + m_file.m_pipe_sentBytes, \
				p_infoClient->reqParser.t_result.body.length() - m_file.m_pipe_sentBytes);
	if (size < 0)
	{
		close(fd);
		m_file.m_pipe_sentBytes = 0;
		return Write::Error;
	}
	m_file.m_pipe_sentBytes+= size;
	if (m_file.m_pipe_sentBytes >= p_infoClient->reqParser.t_result.body.length() )
	{
		close(fd);
		m_file.m_pipe_sentBytes = 0;
		return Write::Complete;
	}
	return Write::Making;
}

void
Response::clearFileEvent()
{
	m_file.fd = -1;
	m_file.size = 0;
	m_file.buffer.clear();
	m_file.m_totalBytes = 0;
	m_file.m_sentBytes = 0;
	m_file.m_pipe_sentBytes = 0;

	fds[0] = -1;
	fds[1] = -1;
	isCgiIng = CGIFL::None;
	m_resMsg.clear();
}
