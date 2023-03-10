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
		return ;
	}

	if (this->reqParser.t_result.method == GET)
	{
		if (_statusCode == AUTO) //autoindex
		{
			this->_statusCode = 200;
			startAutoindex();
			if (this->_statusCode >= 400)
				openErrorResponse(_statusCode);
			else if (_statusCode == INDEX)
			{
				this->_statusCode = 200;
				path = path.substr(0, path.length() - 1);
				startShowFile();
			}
			return ;
		}
		else if (_statusCode == REDIRECTION)
		{
			this->_statusCode = 302;
			startRedirection();
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

	if (cgiFinder(reqParser.t_result.target) != "" || this->reqParser.t_result.method == POST)
	{
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
			close(m_file.inFds[0]);
			close(m_file.outFds[1]);
		
			isCgi = true;
			status = Res::Making;
		}
	}
	else if (this->reqParser.t_result.method == DELETE)
	{
		std::string src = getCwdPath() + reqParser.t_result.target;

		struct stat ss;
		stat(src.c_str(), &ss);
		if (S_ISDIR(ss.st_mode) == true)
		{
			if (deleteDir(src) != true)
				return ;
		}
		else if(access(src.c_str(), W_OK) != -1)
		{
			size_t sub;
			if ((sub = src.rfind(".")) != std::string::npos)
			{
				if (checkForbiddenFile(src.substr(sub)) == true)
					return ;
			}
			if (unlink(src.c_str()) == -1)
			{
				_statusCode = 500;
				openErrorResponse(_statusCode);
				return ;
			}
		}
		else
		{
			switch (errno)
			{
				case EACCES:
					_statusCode = 403;
					break ;
				case ENOENT:
					_statusCode = 404;
					break ;
				default:
					_statusCode= 500;
			}
			openErrorResponse(_statusCode);
			return ;
		}
		_statusCode = 200;
		initHeader();
		makeResult();
	}
}

bool
Client::checkForbiddenFile(std::string src)
{
	if (src == ".cpp" || src == ".hpp")
	{
		_statusCode = 403;
		openErrorResponse(_statusCode);
		return true;
	}
	return false;
}

bool 
Client::deleteDir(std::string path)
{
	DIR *dir;
	if ((dir = opendir(path.c_str())) != NULL)
	{
		if (path[path.length() - 1] != '/')
			path += "/";
		struct dirent *dirent = NULL;
		while (true)
		{
			dirent = readdir(dir);
			if (!dirent)
				break;
			if (strcmp(dirent->d_name, ".") == SUCCESS || strcmp(dirent->d_name, "..") == SUCCESS)
				continue;
			if (dirent->d_type == DT_DIR)
			{
				if (deleteDir(path + dirent->d_name) != true)
					return (false);
			}
			else
			{
				size_t sub;
				std::string dName = dirent->d_name;
				if ((sub = dName.rfind(".")) != std::string::npos)
				{
					if (checkForbiddenFile(dName.substr(sub)) == true)
						return (false);
				}
				if (unlink((path + dName).c_str()) == -1)
				{
					_statusCode = 500;
					openErrorResponse(_statusCode);
					return (false);
				}
			}
		}
		rmdir(path.c_str());
		return (true);
	}
	_statusCode = 500;
	openErrorResponse(_statusCode);
	return (false);
}

char **
Client::initEnv(void)
{
	std::map<std::string, std::string> env_map;

	env_map["AUTH_TYPE"] = ""; 
	if (this->reqParser.t_result.header.find("Content-Length") != this->reqParser.t_result.header.end())
		env_map["CONTENT_LENGTH"] = reqParser.t_result.header.at("Content-Length");
	if (this->reqParser.t_result.header.find("Content-Type") != this->reqParser.t_result.header.end())
		env_map["CONTENT_TYPE"] = reqParser.t_result.header.at("Content-Type");
	env_map["UPLOAD_PATH"] = getCwdPath() + "/database/";
	env_map["REQUEST_METHOD"] = getMethod(reqParser.t_result.method);
	env_map["QUERY_STRING"] = reqParser.t_result.query;
	env_map["REMOTE_ADDR"] = ptr_server->m_ipAddress;
	env_map["REMOTE_USER"] = "";
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
		m_file.fd = fd;
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
		for(size_t i = 0; i < it->second.size(); i++)
		{
			if (it->second[i] == errorCode)
			{
				errorPath = it->first;
				this->_statusCode = isValidTarget(errorPath);
				if (this->_statusCode == 200)
				{
					this->_statusCode = errorCode;
					openfile(errorPath);
					return ;
				}
				break;
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
	if (isCookie == false)
		m_resMsg += "Set-Cookie : " + generateCookie() + CRLF;
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

std::string
Client::generateCookie()
{
	std::string cookie = "";
	std::string sessionID = "session_id";
	std::string sessionIdVal = "webserv_" + generate_random_string(10);
	cookie += sessionID;
	cookie += "=";
	cookie += sessionIdVal;
	cookie += "; Path=/; ";
	cookie += "Secure; HttpOnly";
	return (cookie);
}

std::string
Client::generate_random_string(int length)
{
	std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string result = "";
	std::mt19937 generator(time(0));
	std::uniform_int_distribution<int> distribution(0, characters.size() - 1);

	for (int i = 0; i < length; i++)
	{
		result += characters[distribution(generator)];
	}
	return result;
}

void
Client::startResponse()
{
	initResponse();
	makeResult();
}

void
Client::startAutoindex()
{
	std::string body;

	initHeader();
	setContentType("text/html");

	DIR *dir;
	if ((dir = opendir(path.c_str())))
	{
		std::string index = "Index of  " +  m_file.srcPath;
		body = "<!DOCTYPE html><html><head>    <title> " + index + "</title></head><body bg color='white'><h1>" + index +" </h1><hr>  <pre>\n";
		struct dirent *dirent = NULL;
		while (true)
		{
			dirent = readdir(dir);
			if (!dirent)
				break;
			if (strcmp(dirent->d_name, ".") == SUCCESS || strcmp(dirent->d_name, "..") == SUCCESS)
				continue;

			body += "    <a href= " + m_file.srcPath  + dirent->d_name +"/" + ">" + dirent->d_name +"</a><br>";
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
			case ENOTDIR:
				_statusCode = INDEX;
				break ;
			case EACCES:
				_statusCode = 403;
				break ;
			case ENOENT:
				_statusCode = 404;
				break ;
			default:
				_statusCode = 500;
		}
	}
}

void
Client::startRedirection()
{
	setStatusMsg(_statusMap[getStatusCode()]);
	m_resMsg += getHttpVersion() + " " + std::to_string(getStatusCode())  + " " + getStatusMsg() + CRLF;
	m_resMsg += "Location : " + m_file.srcPath + "\r\n";
	m_resMsg += "Content-type : text/html;charset=UTF-8\r\n";
	m_resMsg += "Content-Length : 0\r\n";
	m_resMsg += "Date : " + getDate() + CRLF;
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

int
Client::sendResponse()
{
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

void 
Client::doubleToSingleSlash(std::string &target)
{
	size_t idx;
	while (true)
	{
		idx = target.find("//");
		if (idx == std::string::npos)
			break;
		else
			target.replace(idx, 2, "/");
	}
}

int
Client::isValidTarget(std::string &target)
{

	if (target == "/home")
		target = "/";
	doubleToSingleSlash(target);
	if (target.find(getCwdPath().c_str(), 0, getCwdPath().length()) != std::string::npos)
	{
		m_file.srcPath = target;
		return (200);
	}
	std::string cgiPath;
	if ((cgiPath =  cgiFinder(target)) != "")
	{
		m_file.srcPath = this->getCwdPath() + "/" + cgiPath + target;
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

				if (it->second.returnType == 301 && it->second.returnRoot != "")
				{
					m_file.srcPath = it->second.returnRoot;
					return (REDIRECTION);
				}
				if (it->second.index.size() > 0 )
				{
					m_file.srcPath = path + "/" +  it->second.index[0];
					return (200);
				}
				else if (this->status != Res::Error && it->second.autoListing == true)
				{
					if ( it->second.root != "/")
						m_file.srcPath = "/" + it->second.root + "/";
					else
						m_file.srcPath = it->second.root;
					return (AUTO);
				}
				else
					return (openDirectory(target));
			}
		}
		if (checkAutoListing())
		{
			path = getCwdPath() + reqParser.t_result.target;
			m_file.srcPath = reqParser.t_result.target;
			return (AUTO);
		}
		if (reqParser.t_result.method == DELETE)
		{
			path = getCwdPath() + reqParser.t_result.target;
			m_file.srcPath = reqParser.t_result.target;
			return (200);
		}
	}
	if (m_file.srcPath  != "")
	{
		m_file.srcPath =  this->getCwdPath() +  "/default.html";
		return (200);
	}

	return (404);
}

int
Client::checkAutoListing()
{
	if ((reqParser.t_result.target.rfind("/")) == reqParser.t_result.target.size() - 1)
	{
		return 1;
	}
	return 0;
}

int
Client::checkDeletePath()
{
	size_t sub;
	std::string subQuery = reqParser.t_result.target;
	if ((sub = subQuery.find('?') ) != std::string::npos)
		subQuery = subQuery.substr(0, sub);
	
	if (reqParser.t_result.method == DELETE)
		return 1;
	return 0;
	

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
	ssize_t size = read(fd, buffer, BUFFER_SIZE);

	if (size < 0)
		return File::Error;
	m_file.buffer += std::string(buffer, size);
	m_file.size += size;
	if (size < BUFFER_SIZE)
		return File::Complete;
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
	this->autoIndexPath = "";
}

int
Client::writePipe(int fd)
{
	size_t size;

	size = write(fd, this->reqParser.t_result.body.c_str() + m_file.m_pipe_sentBytes,
				 this->reqParser.t_result.body.length() - m_file.m_pipe_sentBytes);
	if (size < 0)
        return Write::Error;
    m_file.m_pipe_sentBytes+= size;
    if (m_file.m_pipe_sentBytes >= this->reqParser.t_result.body.length() )
        return Write::Complete;
    return Write::Making;
}

void
Client::startShowFile()
{
	std::string body;
	body = "<!DOCTYPE html><html><body>";

	size_t subb;
	std::string str = path;
	subb = str.rfind("/");
	str = str.substr(subb);
	body += "<a href=\"" + path  + "\" download>"+ str +"</a></body></html>";

	setBody(body);
	setContentLength(body.length());
	makeResult();
}

void
Client::clearClient()
{
	clearResInfo();
	clearResponseByte();
	clearFileEvent();
	reqParser.clearRequest();
	if (isCgi == true)
	{
		isCgi = false;
		cgiOutPath = "";
		cgiOutTarget = "";
	}
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
