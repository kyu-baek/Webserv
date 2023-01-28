#include "Client.hpp"

void Client::openResponse()
{
	std::string cwdPath = this->getCwdPath();
	std::string srcPath = "";
	std::string execPath = "";

	this->_statusCode = isValidTarget(this->reqParser.t_result.target);
	if (this->_statusCode >= 400)
	{
		// send Error msg;
		std::cerr << "	ERROR : INVALID TARGET : " << reqParser.t_result.target <<"\n";
		return;
	}

	if (this->reqParser.t_result.method == GET)
	{
		std::cerr << "GET RESPONSE\n";
		std::cerr << "statusRes :" << this->_statusCode << "\n";
		if (this->_statusCode == 200)
		{
			srcPath = cwdPath + "/www/statics" + this->reqParser.t_result.target;
			this->status = Res::Making;
			int fd = -1;
			struct stat ss;
			std::cout << "srcPath : " << srcPath << std::endl;
			if (stat(srcPath.c_str(), &ss) == -1 || S_ISREG(ss.st_mode) != true || (fd = open(srcPath.c_str(), O_RDONLY)) == -1)
				this->_statusCode = 500;
			else
			{
				m_file.fd = fd;
				m_file.srcPath = srcPath;
				this->status = Res::Making;
			}
		}
		if (this->_statusCode == 404 || this->_statusCode == 500)
		{
			std::cerr << "	NO FILE FOUND\n";
			// 404 response
		}
	}

	if (this->reqParser.t_result.method == POST || this->reqParser.t_result.method == DELETE)
	{
		cwdPath = this->getCwdPath();
		execPath = getCwdPath() + "/www/cgi-bin" + reqParser.t_result.target;
		std::cout << "METHOD : " << reqParser.t_result.method <<"\n";
		std::cout << "execPath : " << execPath << std::endl;

		if (pipe(m_file.inFds) == -1)
			std::cerr << "ERROR: pipe\n";
		if (pipe(m_file.outFds) == -1)
		{
			close(m_file.inFds[0]);
			close(m_file.inFds[1]);
			std::cerr << "ERROR: pipe\n";
		}

		m_file.pid = fork();
		if (m_file.pid == -1)
		{
			close(m_file.inFds[0]);
			close(m_file.inFds[1]);

			close(m_file.outFds[0]);
			close(m_file.outFds[1]);
			std::cerr << "ERROR: return 500\n";
		}

		if (m_file.pid == 0)
		{
			close(m_file.inFds[1]);
			dup2(m_file.inFds[0], STDIN_FILENO);
			close(m_file.inFds[0]);

			close(m_file.outFds[0]);
			dup2(m_file.outFds[1], STDOUT_FILENO);
			close(m_file.outFds[1]);

			char **env = initEnv();
			char **args = new char *[sizeof(char *) * 2];
			args[0] = strdup(execPath.c_str());
			args[1] = NULL;

			if (execve(args[0], args, env) == -1)
			{
				std::cerr << errno << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			std::cout << "	This is Parent of POST : \n";
			
			close(m_file.inFds[0]);
			close(m_file.outFds[1]);

			// waitpid(pid, NULL, WNOHANG);

			isCgi = true;
			status = Res::Making;
		}
	}

	// if (this->reqParser.t_result.method == DELETE)
	// {
	// 	cwdPath = this->getCwdPath();
	// 	execPath = getCwdPath() + "/www/cgi-bin" + reqParser.t_result.target;
	// 	std::cout << "execPath : " << execPath << std::endl;

	// 	if (pipe(m_file.outFds) == -1)
	// 	{
	// 		std::cerr <<"ERROR: pipe\n";
	// 	}

	// 	m_file.pid = fork();
	// 	if (m_file.pid == -1)
	// 	{
	// 		close(m_file.outFds[0]);
	// 		close(m_file.outFds[1]);
	// 		std::cerr <<"ERROR: return 500\n";
	// 	}

	// 	if (m_file.pid == 0)
	// 	{
	// 		close(m_file.outFds[0]);
	// 		dup2(m_file.outFds[1], STDOUT_FILENO);
	// 		close(m_file.outFds[1]);

	// 		char **env = initEnv();
	// 		char **args = new char *[sizeof(char *) * 2];
	// 		args[0] = strdup(execPath.c_str());
	// 		args[1] = NULL;

	// 		if (execve(args[0], args, env) == -1)
	// 		{
	// 			std::cerr << errno << std::endl;
	// 			exit(EXIT_FAILURE);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		close(m_file.outFds[1]);
	// 		isCgi = true;
	// 		status = Res::Making;
	// 	}
	// }

	// if (this->reqParser.t_result.method == DELETE)
	// {
	// 	int pid = fork();
	// 	if (pid == 0)
	// 	{
	// 		std::cerr << "\n\nTHIS IS DELETE METHOD \n\n";
	// 		std::string uploadPath = cwdPath + "/database/";

	// 		/* all files delete in uploaded dir */
	// 		DIR *dir = opendir(uploadPath.c_str());
	// 		struct dirent *dirent = NULL;
	// 		while (true)
	// 		{
	// 			dirent = readdir(dir);
	// 			if (!dirent)
	// 				break;
	// 			// std::cout << dirent->d_name << "\n";
	// 			if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, ".."))
	// 			{
	// 				std::string delPath = uploadPath + dirent->d_name;
	// 				unlink(delPath.c_str());
	// 			}
	// 		}
	// 	}
}

char **
Client::initEnv(void)
{
	std::map<std::string, std::string> env_map;
	env_map["AUTH_TYPE"] = "";
	env_map["CONTENT_LENGTH"] = reqParser.t_result.header.at("content-length");
	env_map["CONTENT_TYPE"] = reqParser.t_result.header.at("content-type");
	// std::cout << "\n IN CGI initEnv -> CONTENT_TYPE = " << reqParser.t_result.header.at("content-type") << "\n\n";
	env_map["UPLOAD_PATH"] = getCwdPath() + "/database/";
	env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
	env_map["REQUEST_METHOD"] = reqParser.t_result.method;
	env_map["QUERY_STRING"] = reqParser.t_result.query;
	env_map["REMOTE_ADDR"] = ptr_server->m_ipAddress + std::to_string(ptr_server->m_port);
	env_map["REMOTE_USER"] = "";
	env_map["SERVER_NAME"] = this->reqParser.t_result.host + ":" + this->reqParser.t_result.port;
	env_map["SERVER_PORT"] = this->reqParser.t_result.port;
	env_map["SERVER_PROTOCOL"] = this->reqParser.t_result.version;
	env_map["SERVER_SOFTWARE"] = "webserv/1.0";
	env_map["PATH_INFO"] = reqParser.t_result.target;

	// this->set_cgi_env_path(env_map, this->target_info.url);
	// this->set_cgi_custom_env(env_map, *(this->req_info.header_map));
	char **cgi_env = new char *[sizeof(char *) * env_map.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::iterator iter = env_map.begin(); iter != env_map.end(); iter++)
	{
		cgi_env[i] = strdup((iter->first + "=" + iter->second).c_str());
		i++;
	}

	cgi_env[env_map.size()] = NULL;
	return (cgi_env);
}

void Client::initResponse()
{
	setStatusCode(reqParser.t_result.status);
	setStatusMsg(_statusMap[getStatusCode()]);
	setDate();
	if (this->reqParser.t_result.close == true)
		setConnection("close");
	else
		setConnection("keep-alive");
	setContentType("text/html");
	setTransferEncoding("identity");
	setContentLength(m_file.buffer.size());
	setBody(m_file.buffer);
}

void Client::startResponse()
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
	m_resMsg += m_file.buffer;
	m_totalBytes = m_resMsg.size();
}

const char *
Client::getSendResult() const
{
	return (this->m_resMsg.c_str() + this->m_sentBytes);
}

size_t
Client::getSendResultSize() const
{
	return (this->m_totalBytes - this->m_sentBytes);
}

size_t
Client::changePosition(int n)
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

int Client::sendResponse()
{
	std::cout << "SEND DATA\n"
			  << getSendResult();
	size_t n = send(m_clientFd, getSendResult(), getSendResultSize(), 0);

	if (n < 0)
		return Send::Error;
	else if (changePosition(n) != 0)
		return Send::Making;
	else
		return Send::Complete;
}

void Client::clearResponseByte()
{
	m_resMsg.clear();
	m_sentBytes = 0;
	m_totalBytes = 0;
}

int Client::isValidTarget(std::string &target)
{
	if (target == "")
		return 404;
	if (target == "/" || target == "/home")
		target = "index.html";
	else if (target == "/submit")
		target = "submit.html";
	else if (target == "/delete")
		target = "delete.html";
	else if (target == "/upload")
		target = "upload.html";
	else if (target == "/server")
		target = "server.html";
	else if (target == "/post.py")
		target = "post.py";
	else if (target == "/upload.py")
		target = "upload.py";
	else if (target == "/submit.py")
		target = "submit.py";
	else if (target == "/delete.py")
		target = "delete.py";
	else if (target == "/image.jpeg")
		target = "image.jpeg";

	if (target == "/favicon.ico")
		std::cout << "\n-->FAVICON REQUESTED \n";

	std::string srcPath;
	srcPath = this->getCwdPath() + "/www/statics";
	if (this->reqParser.t_result.method == POST || this->reqParser.t_result.method == DELETE)
		srcPath = this->getCwdPath() + "/www/cgi-bin";

	std::cout << "SROUCE PATH : " << srcPath << std::endl;
	DIR *dir = opendir(srcPath.c_str());
	struct dirent *dirent = NULL;
	while (true)
	{
		dirent = readdir(dir);
		if (!dirent)
			break;
		if (strcmp(dirent->d_name, (target).c_str()) == SUCCESS)
		{
			std::cout << "\n[!]SRC FOUND \n";
			(target).insert(0, "/");
			return (200);
		}
	}
	return (404);
}

int Client::readFile(int fd)
{
	char buffer[BUFFER_SIZE + 1];

	memset(buffer, 0, sizeof(buffer));
	// std::cout << "reading\n";
	ssize_t size = read(fd, buffer, BUFFER_SIZE);
	std::cout << "size : " << size << std::endl;
	if (size < 0)
	{
		// std::cout << "size < 0" << std::endl;
		return File::Error;
	}
	// vector<char> 로 바꾸고 미리 파일 크기 만큼   해서 용량을 미리 확보한다.
	m_file.buffer += std::string(buffer, size);
	m_file.size += size;
	// std::cout << m_file.size << "<<<<< SIZE_READ\n";
	if (size < BUFFER_SIZE)
	{
		// std::cout << "size < BUFFER_SIZE" << std::endl;
		// std::cout << m_file.buffer << std::endl;
		// close(fd);
		// _fdMap.erase(fd);
		return File::Complete;
	}
	return File::Making;
}

void Client::clearFileEvent()
{
	m_file.fd = -1;
	m_file.size = 0;
	m_file.buffer = "";
	m_file.m_sentBytes = 0;
	m_file.m_totalBytes = 0;
	m_file.m_pipe_sentBytes = 0;
	m_file.inFds[0] = -1;
	m_file.inFds[1] = -1;
	m_file.outFds[0] = -1;
	m_file.outFds[1] =-1;
	m_file.isFile = 0;
	m_file.srcPath ="";
}

int Client::writePipe(int fd)
{
	size_t size;

	size = write(fd, this->reqParser.t_result.body.c_str() + m_file.m_pipe_sentBytes,
				 this->reqParser.t_result.body.length() - m_file.m_pipe_sentBytes);
	std::cout << "Write size : " << size << std::endl;
	if (size < 0)
	{
		return Write::Error;
	}
	m_file.m_pipe_sentBytes += size;
	if (m_file.m_pipe_sentBytes >= this->reqParser.t_result.body.length())
	{
		std::cout << "PIPE WRITE COMPLETE\n";
		return Write::Complete;
	}
	return Write::Making;
}
