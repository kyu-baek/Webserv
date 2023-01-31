#include "Client.hpp"
#include <algorithm>

void
Client::openResponse()
{
	this->status = Res::Making;
	this->_statusCode = isValidTarget(this->reqParser.t_result.target);
	if (this->_statusCode >= 400)
	{
		openErrorResponse(_statusCode);
		std::cerr << "	ERROR : INVALID TARGET\n";
		return ;
	}

	std::cout << "statusRes :" << this->_statusCode << "\n";
	if (this->reqParser.t_result.method == GET)
	{
		if (_statusCode == AUTO) //autoindex
		{
			std::cout << "  AUTOINDEX \n";
			this->_statusCode = 200;
			starAutoindex();
			if (this->_statusCode >= 400)
			{
				openErrorResponse(_statusCode);
				std::cerr << "	ERROR : INVALID TARGET\n";
			}
			return ;
		}

		int fd = -1;
		struct stat ss;
		if (stat(m_file.srcPath.c_str(), &ss) == -1 || S_ISREG(ss.st_mode) != true || (fd = open(m_file.srcPath.c_str(), O_RDONLY)) == -1)
		{
			this->_statusCode = 500;
			openErrorResponse(_statusCode);
			std::cerr << "	NO FILE FOUND\n";
			return ;
		}
		else
			m_file.fd = fd;
	}

	if (this->reqParser.t_result.method == POST || this->reqParser.t_result.method == DELETE)
	{
		std::cout << "m_file.srcPath  : " <<m_file.srcPath  << std::endl;
		/*
			file open logic!
		*/
		char **env = initEnv();
		if (pipe(m_file.inFds) == -1)
			std::cerr <<"ERROR: pipe\n";
		if (pipe(m_file.outFds) == -1)
		{
			close(m_file.inFds[0]);
			close(m_file.inFds[1]);
			std::cerr <<"ERROR: pipe\n";
		}

		m_file.pid = fork();
		if (m_file.pid == -1)
		{
			close(m_file.inFds[0]);
			close(m_file.inFds[1]);

			close(m_file.outFds[0]);
			close(m_file.outFds[1]);
			std::cerr <<"ERROR: return 500\n";
		}

		if (m_file.pid == 0)
		{

			close(m_file.inFds[1]);
			dup2(m_file.inFds[0], STDIN_FILENO);
			close(m_file.inFds[0]);

			close(m_file.outFds[0]);
			dup2(m_file.outFds[1], STDOUT_FILENO);
			close(m_file.outFds[1]);

			char **arg = new char *[sizeof(char *) * 3];

			std::string str = getExecvePath();

			arg[0] = strdup(str.c_str()); //예시 "/usr/bin/python3"
			arg[1] = strdup(m_file.srcPath.c_str()); //실행할 파일의 절대경로.
			arg[2] = NULL;

			if (execve(arg[0], arg, env) == -1)
			{
				std::cerr << "ERRRRRRRRRR Errno is : \n";
				std::cerr << errno << std::endl;
				exit(EXIT_SUCCESS);
			}
			exit(0);
		}
		else
		{
			std::cout << "	This is Parent of POST : \n";
			std::cout << "222\n";
			std::cout << "inFds[0] : " << m_file.inFds[0] << " inFds[1] : " << m_file.inFds[1] <<std::endl;
			std::cout << "outFds[0] : " << m_file.outFds[0] << " outFds[1] : " << m_file.outFds[1] <<std::endl;

			close(m_file.inFds[0]);
			close(m_file.outFds[1]);

			// waitpid(pid, NULL, WNOHANG);

			isCgi = true;
			status = Res::Making;
		}

	}
}

char **
Client::initEnv(void)
{
	std::map<std::string, std::string> env_map;

	env_map["AUTH_TYPE"] = ""; // 인증과정 없으므로 NULL
	env_map["CONTENT_LENGTH"] = reqParser.t_result.header.at("Content-Length");
	env_map["CONTENT_TYPE"] = reqParser.t_result.header.at("Content-Type");
	env_map["UPLOAD_PATH"] = getCwdPath() + "/database/";
	env_map["REQUEST_METHOD"] = getMethod(reqParser.t_result.method);
	env_map["QUERY_STRING"] = reqParser.t_result.query;
	env_map["REMOTE_ADDR"] = ptr_server->m_ipAddress;
	env_map["REMOTE_USER"] = ""; // 인증과정 없으므로 NULL
	env_map["SERVER_NAME"] = ptr_server->m_ipAddress;
	env_map["SERVER_PORT"] = std::to_string(ptr_server->m_port);
	env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
	env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
	env_map["SERVER_SOFTWARE"] = "webserv/1.1";
	env_map["SCRIPT_NAME"] = "webserv/1.1";
	env_map["PATH_INFO"] = getExecvePath();
	env_map["REQUEST_URI"] = getExecvePath();

	std::map<std::string, std::string>::iterator it;
	for (it = reqParser.t_result.header.begin(); it != reqParser.t_result.header.end(); it++)
	{
		std::string::size_type sub;
		std::string str = it->first;
		for (unsigned long i = 0; i < str.size(); i++) { str[i] = std::toupper(str[i]); }
		while ((sub = str.rfind("-")) != std::string::npos) { str = str.replace(sub, 1, 1, '_'); }
		str.insert(0, "HTTP_");
		env_map[str] = it->second;
	}
	// std::cout << "	ENV!\n";
	// std::map<std::string, std::string>::iterator it2;
	// for (it2 = env_map.begin() ; it2 != env_map.end(); it2++)
	// 	std::cout << it2->first << " : [" << it2->second << "]\n";

	char **cgi_env = new char *[sizeof(char *) * env_map.size() + 1];
	int i = 0;
	for(std::map<std::string, std::string>::iterator iter = env_map.begin(); iter != env_map.end(); iter++)
	{
		cgi_env[i] = strdup((iter->first + "=" + iter->second).c_str());
		i++;
	}

	cgi_env[i] = NULL;
	return (cgi_env);
}

void
Client::openfile(std::string targetPath)
{
	std::string tmpPath = path  + targetPath;
	std::cout << "tmpPath : " << tmpPath << std::endl;
	int fd;
	struct stat ss;
	if (stat(tmpPath.c_str(), &ss) == -1 || S_ISREG(ss.st_mode) != true ||
		(fd = open(tmpPath.c_str(), O_RDONLY)) == -1)
	{
		this->status = Res::Error;
		this->_statusCode = 500;
		openErrorResponse(_statusCode);
	}
	else
	{
		std::cout <<"file size = "<< ss.st_size << std::endl;
		std::cout << "fd = "<< fd<<std::endl;
		m_file.fd = fd;
		std::cout << "_statusCode : " << getStatusCode() << std::endl;
		this->status = Res::Making;
	}
}

void
Client::openErrorResponse(int errorCode)
{
	this->status = Res::Error;
	std::string errorPath = "";
	this->_statusCode = errorCode;

	std::map<std::string, std::vector<int> >::iterator it;
	for (it = ptr_server->m_errorPages.begin(); it != ptr_server->m_errorPages.end(); it++)
	{
		for (unsigned int i = 0; i < it->second.size(); i++ )
		{
			if (it->second.at(i) == errorCode)
			{
				errorPath = it->first;
				this->_statusCode = isValidTarget(errorPath);
				if (this->_statusCode == 200)
				{
					this->_statusCode = errorCode;
					openfile(errorPath);
					return ;
				}
			}
		}
	}
	this->_statusCode = errorCode;
	path  = this->getCwdPath();
	errorPath = "/default.html";
	openfile(errorPath);
}

void
Client::initResponse()
{
	std::map<std::string, std::string> mimeMap = initMimeMap();
	std::map<std::string, std::string>::iterator it;

	setStatusCode(getStatusCode());
	setStatusMsg(_statusMap[getStatusCode()]);
	setDate();

	if (this->reqParser.t_result.close == true)
		setConnection("close");
	else
		setConnection("keep-alive");

	for (it = mimeMap.begin(); it != mimeMap.end(); ++it)
	{
		std::string srcTarget = this->reqParser.t_result.target;
		std::string extension("");
		size_t pos = srcTarget.find_last_of(".");
		if (pos != std::string::npos)
			extension = srcTarget.substr(pos);
		if (it->first == extension)
			setContentType(it->second);
	}

	setTransferEncoding("identity");
	setContentLength(m_file.buffer.size());
	setBody(m_file.buffer);

}

void
Client::initHeader()
{
	setServer("webserv");
	setStatusMsg(_statusMap[getStatusCode()]);
	setDate();
	if (this->reqParser.t_result.close == true)
		setConnection("close");
	else
		setConnection("keep-alive");
	setAcceptRange("bytes");
}

void
Client::makeResult()
{
	m_resMsg += getHttpVersion() + " " + std::to_string(getStatusCode())  + " " + getStatusMsg() + CRLF;
	m_resMsg += "Connection : " + getConnection() + CRLF;
	m_resMsg += "Date : " + getDate() + CRLF;
	m_resMsg += "Server : " + getServer() + CRLF;
	m_resMsg += "Content-type : " + getContentType() + CRLF;
	m_resMsg += "Transfer-Encoding : " + getTransferEncoding() + CRLF;
	m_resMsg += "Content-Length : " + std::to_string(getContentLength()) + CRLF;
	m_resMsg += "\n";
	m_resMsg += getResponseBody();
	m_totalBytes = m_resMsg.size();
}

void
Client::startResponse()
{
	initResponse();
	makeResult();
}

void
Client::starAutoindex()
{
	std::string body;

	initHeader();
	setContentType("text/html");

	DIR *dir;
	if ((dir = opendir(path.c_str())))
	{

		body = "<html><head>    <title>Index of " + path + "</title></head><body bg color='white'> <hr>  <pre>";
		struct dirent *dirent = NULL;
		while (true)
		{
			dirent = readdir(dir);
			if (!dirent)
				break;
			if (strcmp(dirent->d_name, ".") == SUCCESS || strcmp(dirent->d_name, "..") == SUCCESS)
				continue;
			body += "    <a href= " + path + "/" + dirent->d_name + ">" + dirent->d_name + "</a><br>";
		}
		closedir(dir);
		body += "</pre>  <hr></body></html>";
		setBody(body);
		setContentLength(body.length());
		makeResult();
	}
	else 
	{
		switch (errno)
		{
			case EACCES:
				_statusCode = 403;
			case ENOENT:
				_statusCode = 404;
			default:
				_statusCode = 500;
		}
	}

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

int
Client::sendResponse()
{
	std::cout << "SEND DATA\n"<< getSendResult();
	size_t n = send(m_clientFd, getSendResult(), getSendResultSize(), 0);

	if (n < 0)
		return Send::Error;
	else if (changePosition(n) != 0)
		return Send::Making;
	else
		return Send::Complete;
}

void
Client::clearResponseByte()
{
	m_resMsg.clear();
	m_sentBytes = 0;
	m_totalBytes = 0;
}

std::string
Client::getExecvePath()
{
	std::string str = reqParser.t_result.target;
	size_t sub;

	if ((sub = str.rfind(".")) != std::string::npos)
	{
		str = str.substr(sub);
		return ptr_server->m_cgi.find(str)->second.execPath;
	}
	return "";
}

std::string
Client::cgiFinder(std::string target)
{
	std::string str =  target;
	size_t sub;

	if ((sub = str.rfind(".")) != std::string::npos)
		str = str.substr(sub);
	else
		return "";
	if (ptr_server->m_cgi.find(str) == ptr_server->m_cgi.end())
		return "";
	return ptr_server->m_cgi.find(str)->second.root;
}


int
Client::isValidTarget(std::string &target)
{
	std::cout << "target : " <<target << std::endl;
	if (target == "/home")
		target = "/";
	if (target == "/favicon.ico")
		std::cout << "\n-->FAVICON REQUESTED \n";

	std::string cgiPath;
	if ((cgiPath =  cgiFinder(target)) != "")
	{
		m_file.srcPath = this->getCwdPath() + "/" + cgiPath + target;
		std::cout << "cgi!! : " << path << "\n";
		return (200);
	}
	if (target.compare(0, sizeof("/database/") - 1, "/database/") == SUCCESS)
	{
		m_file.srcPath = this->getCwdPath() + target;
		std::cout << "download!! : " << path << "\n";
		return (200);
	}
	else
	{
		std::map<std::string, Location>::iterator it = ptr_server->m_location.begin();
		for (; it != ptr_server->m_location.end(); it++)
		{
			if (it->first == target)
			{
				path = this->getCwdPath() + "/"+ it->second.root;
				std::cout << "!!path : " << path << std::endl;
				std::cout << "t->second.index.size() :" << it->second.index.size()  << "\n";
				if (it->second.index.size() > 0 )
				{
					m_file.srcPath = path + "/" +  it->second.index[0];
					std::cout << "!!target : " << target << std::endl;
					return (200);
				}
				else if (this->status != Res::Error && it->second.autoListing == true)
				{
					m_file.srcPath = path + "/";
					return (AUTO);
				}
				else
					return (openDirectory(target));
			}
		}
	}
	if (m_file.srcPath  != "")
	{
		std::cout << "path nothing \n";
		m_file.srcPath =  this->getCwdPath() +  "/default.html";
		return (200);
	}
	return (404);
}

int
Client::openDirectory(std::string &target)
{
	size_t sub;

	if (this->status == Res::Error)
	{
		size_t sub;
		std::string str = target;
		if ((sub = str.rfind(".")) != std::string::npos)
		{
			str = str.substr(sub);
		}
		target = std::to_string(_statusCode) + str;
	}
	else if ((sub = target.rfind("/")) != std::string::npos)
		target = target.substr(sub + 1);

	std::cout << "SROUCE target : " << target << std::endl;

	DIR *dir;
	if ((dir = opendir(path.c_str())))
	{
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
				m_file.srcPath = path + target;
				closedir(dir);
				return (200);
			}
		}
		closedir(dir);
		return (404);
	}
	else
	{
		switch (errno)
		{
			case EACCES:
				return 403;
			case ENOENT:
				return 404;
			default:
				return 500;
		}
	}
}


int
Client::readFile(int fd)
{
	char buffer[BUFFER_SIZE + 1];

	memset(buffer, 0, sizeof(buffer));
	//std::cout << "reading\n";
	ssize_t size = read(fd, buffer, BUFFER_SIZE);
	std::cout << "size : " << size << std::endl;
	if (size < 0)
	{
		std::cout << "size < 0" << std::endl;
		return File::Error;
	}
	//vector<char> 로 바꾸고 미리 파일 크기 만큼   해서 용량을 미리 확보한다.
	m_file.buffer += std::string(buffer, size);
	m_file.size += size;
	std::cout << m_file.size << "<<<<< SIZE_READ\n";
	if (size < BUFFER_SIZE)
	{
		std::cout << "size < BUFFER_SIZE" << std::endl;
		std::cout << m_file.buffer << std::endl;
		// close(fd);
		// _fdMap.erase(fd);
		return File::Complete;
	}
	return File::Making;
}

void
Client::clearFileEvent()
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
	this->path = "";
}

int
Client::writePipe(int fd)
{
    size_t size;

    size = write(fd, this->reqParser.t_result.body.c_str() + m_file.m_pipe_sentBytes, \
                this->reqParser.t_result.body.length() - m_file.m_pipe_sentBytes);
    std::cout << "Write size : " << size << std::endl;
	if (size < 0)
    {
        return Write::Error;
    }
    m_file.m_pipe_sentBytes+= size;
    if (m_file.m_pipe_sentBytes >= this->reqParser.t_result.body.length() )
    {
		std::cout << "PIPE WRITE COMPLETE : \n[";
		std::cout << this->reqParser.t_result.body << "]"<< std::endl;
        return Write::Complete;
    }
    return Write::Making;
}

std::map<std::string, std::string>
Client::initMimeMap()
{
	std::map<std::string, std::string> mimeTypes;

	mimeTypes[".aac"] = "audio/aac";
	mimeTypes[".abw"] = "application/x-abiword";
	mimeTypes[".arc"] = "application/octet-stream";
	mimeTypes[".avi"] = "video/x-msvideo";
	mimeTypes[".azw"] = "application/vnd.amazon.ebook";
	mimeTypes[".bin"] = "application/octet-stream";
	mimeTypes[".bz"] = "application/x-bzip";
	mimeTypes[".bz2"] = "application/x-bzip2";
	mimeTypes[".csh"] = "application/x-csh";
	mimeTypes[".css"] = "text/css";
	mimeTypes[".csv"] = "text/csv";
	mimeTypes[".doc"] = "application/msword";
	mimeTypes[".epub"] = "application/epub+zip";
	mimeTypes[".gif"] = "image/gif";
	mimeTypes[".htm"] = "text/html";
	mimeTypes[".html"] = "text/html";
	mimeTypes[".ico"] = "image/x-icon";
	mimeTypes[".ics"] = "text/calendar";
	mimeTypes[".jar"] = "Temporary Redirect";
	mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".jpg"] = "image/jpeg";
	mimeTypes[".js"] = "application/js";
	mimeTypes[".json"] = "application/json";
	mimeTypes[".mid"] = "audio/midi";
	mimeTypes[".midi"] = "audio/midi";
	mimeTypes[".mpeg"] = "video/mpeg";
	mimeTypes[".mpkg"] = "application/vnd.apple.installer+xml";
	mimeTypes[".odp"] = "application/vnd.oasis.opendocument.presentation";
	mimeTypes[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	mimeTypes[".odt"] = "application/vnd.oasis.opendocument.text";
	mimeTypes[".oga"] = "audio/ogg";
	mimeTypes[".ogv"] = "video/ogg";
	mimeTypes[".ogx"] = "application/ogg";
	mimeTypes[".png"] = "image/png";
	mimeTypes[".pdf"] = "application/pdf";
	mimeTypes[".ppt"] = "application/vnd.ms-powerpoint";
	mimeTypes[".rar"] = "application/x-rar-compressed";
	mimeTypes[".rtf"] = "application/rtf";
	mimeTypes[".sh"] = "application/x-sh";
	mimeTypes[".svg"] = "image/svg+xml";
	mimeTypes[".swf"] = "application/x-shockwave-flash";
	mimeTypes[".tar"] = "application/x-tar";
	mimeTypes[".tif"] = "image/tiff";
	mimeTypes[".tiff"] = "image/tiff";
	mimeTypes[".ttf"] = "application/x-font-ttf";
	mimeTypes[".txt"] = "text/plain";
	mimeTypes[".vsd"] = "application/vnd.visio";
	mimeTypes[".wav"] = "audio/x-wav";
	mimeTypes[".weba"] = "audio/webm";
	mimeTypes[".webm"] = "video/webm";
	mimeTypes[".webp"] = "image/webp";
	mimeTypes[".woff"] = "application/x-font-woff";
	mimeTypes[".xhtml"] = "application/xhtml+xml";
	mimeTypes[".xls"] = "application/vnd.ms-excel";
	mimeTypes[".xml"] = "application/xml";
	mimeTypes[".xul"] = "application/vnd.mozilla.xul+xml";
	mimeTypes[".zip"] = "application/zip";
	mimeTypes[".3gp"] = "video/3gpp audio/3gpp";
	mimeTypes[".3g2"] = "video/3gpp2 audio/3gpp2";
	mimeTypes[".7z"] = "application/x-7z-compressed";

	return mimeTypes;
}
