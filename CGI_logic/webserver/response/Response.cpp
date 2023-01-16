/***************************************************/
/* CODED BY JIN H. BANG ===========================*/
/***************************************************/

#include "Response.hpp"
#include "CGI.hpp"
#include <sys/types.h>
#include <dirent.h>

void Response::responseToClient(int clientSocket, InfoClient infoClient)
{
	char cwd[1024];
	getcwd(cwd, 1024);
	std::string cwdPath(cwd);
	std::string execPath = "";
	std::string filePath = "";
	std::string resMsg = "";
	if (infoClient.req.t_result.method == GET)
	{
		resMsg = makeResponseGET(infoClient);
		std::cout << "		------result msg------- : \n"
				  << resMsg << "\n";
	}
	else if (infoClient.req.t_result.method == DELETE)
	{
		int pid = fork();
		if (pid == 0)
		{
			std::cerr << "\n\nTHIS IS DELETE METHOD \n\n";
			std::string uploadPath = cwdPath + "/uploaded/";

			/* all files delete in uploaded dir */
			DIR *dir = opendir(uploadPath.c_str());
			struct dirent *dirent = NULL;
			while (true)
			{
				dirent = readdir(dir);
				if (!dirent)
					break;
				// std::cout << dirent->d_name << "\n";
				if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, ".."))
				{
					std::string delPath = uploadPath + dirent->d_name;
					unlink(delPath.c_str());
				}
			}
		}
	}
	else if (infoClient.req.t_result.method == POST)
	{
		if (infoClient.req.t_result.target == "/www/cgi-bin/submit.py")
		{
			execPath = cwdPath + "/www/cgi-bin/submit.py";
			filePath = cwdPath + "/submit_out.html";
		}
		else if (infoClient.req.t_result.target == "/www/cgi-bin/upload.py")
		{
			execPath = cwdPath + "/www/cgi-bin/upload.py";
			filePath = cwdPath + "/upload_out.html";
		}

		std::string tmpFilePath = cwdPath + "/tmp_" + std::to_string(clientSocket);

		/* tmp file to save content body */
		int tmpFd = open(tmpFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0744);
		write(tmpFd, infoClient.req.t_result.body.c_str(), infoClient.req.t_result.body.size());
		close(tmpFd);

		/* args for CGI script */
		// char **args = new char *[sizeof(char *) * 3];
		// args[0] = strdup(execPath.c_str());
		// args[1] = NULL;
		char *args[2] = {strdup(execPath.c_str()), NULL};

		/* cgi env setting */
		CGI cgi;
		cgi.initEnvMap(infoClient);

		cgi.envMap.insert(std::pair<std::string, std::string>("UPLOAD_PATH", cwdPath + "/uploaded/"));
		cgi.envMap.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", args[0]));

		// char **cgiEnv = new char *[sizeof(char *) * cgi.envMap.size() + 1]; // delete needed
		char *cgiEnv[cgi.envMap.size() + 1];
		cgiEnv[cgi.envMap.size()] = NULL;
		int i = 0;
		for (std::map<std::string, std::string>::iterator iter = cgi.envMap.begin(); iter != cgi.envMap.end(); ++iter)
		{
			cgiEnv[i] = strdup((iter->first + "=" + iter->second).c_str());
			// std::cout << cgiEnv[i] << "\n";
			i++;
		}

		int fds[2];
		pipe(fds);

		int pid = fork();

		if (pid == 0)
		{
			// std::cerr << "CHILD" << std::endl;
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO);
			close(fds[0]);
			int resFd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0744);
			if (resFd < 0)
				std::cerr << "Error : resFd open()\n";
			dup2(resFd, STDOUT_FILENO);
			close(resFd);

			execve(execPath.c_str(), args, cgiEnv);

			exit(EXIT_SUCCESS);
		}
		else
		{
			int tmpFd = open(tmpFilePath.c_str(), O_RDONLY);
			char buff[1024] = {0};

			// std::cerr << "OPEN" << std::endl;
			int n = read(tmpFd, buff, sizeof(buff));
			close(tmpFd);
			unlink(tmpFilePath.c_str()); // if you want to check tmpfile, delete it.

			if (n < 0)
				std::cerr << "Error : read()\n";

			close(fds[0]);
			write(fds[1], buff, sizeof(buff));
			// std::cerr << "		test\n" << buff << "\n\n";
			close(fds[1]);

			waitpid(pid, NULL, 0);

			/* msg to client */
			resMsg = resMsgHeader(infoClient);
			resMsg += "\n" + resMsgBody(filePath);

			std::cout << " response to client : " << clientSocket << "\n";
			long valWrite = write(clientSocket, resMsg.c_str(), resMsg.size());
			if (valWrite == (long)resMsg.size())
				std::cout << "SERVER RESPONSE SENT\n";
			unlink(filePath.c_str()); // if you want to check output html, delete it
			return;
		}
	}
	else
	{
		resMsg = makeResponseERR();
	}

	std::cout << " response to client : " << clientSocket << "\n";
	long valWrite = write(clientSocket, resMsg.c_str(), resMsg.size());
	if (valWrite == (long)resMsg.size())
		std::cout << "SERVER RESPONSE SENT\n";
}

std::string
Response::makeResponseGET(InfoClient &infoClient)
{
	std::string resMsg;
	char cwd[1024];
	getcwd(cwd, 1024);
	std::string cwdPath(cwd);
	if (infoClient.req.t_result.target == "/home" || infoClient.req.t_result.target == "/")
		resMsg = resMsgHeader(infoClient) + "\n" + resMsgBody(cwdPath + "/resource/static/index.html");
	else if (infoClient.req.t_result.target == "/server")
		resMsg = resMsgHeader(infoClient) + "\n" + resMsgBody(cwdPath + "/resource/static/server.html");
	else if (infoClient.req.t_result.target == "/submit")
		resMsg = resMsgHeader(infoClient) + "\n" + resMsgBody(cwdPath + "/resource/static/submit.html");
	else if (infoClient.req.t_result.target == "/upload")
		resMsg = resMsgHeader(infoClient) + "\n" + resMsgBody(cwdPath + "/resource/static/upload.html");
	else if (infoClient.req.t_result.target == "/delete")
		resMsg = resMsgHeader(infoClient) + "\n" + resMsgBody(cwdPath + "/resource/static/delete.html");
	else
		resMsg = makeResponseERR();
	return (resMsg);
}

std::string
Response::makeResponseERR()
{
	std::stringstream httpRes;
	std::string htmlMsg = "HTTP/1.1 400 Not Found\nContent-Type: text/html; charset=utf-8\nContent-Length: 200\n\n<h1>ERROR</h1> <h2>404 NOT FOUND</h2>";
	return (htmlMsg);
}

std::string
Response::resMsgHeader(InfoClient &infoClient)
{
	std::stringstream header;
	(void)infoClient;
	setStatusCode(200);
	setStatusMsg(_statusMap[200]);
	setConnection("keep-alive");
	if (infoClient.req.t_result.close == true)
		setConnection("close");
	setContentType("text/html");
	// setTransferEncoding("chunked");
	struct stat st;
	std::string resMsg;
	char cwd[1024];
	getcwd(cwd, 1024);
	std::string cwdPath(cwd);
	std::string file = "";
	int contentLen = 0;
	if (infoClient.req.t_result.method == GET)
	{
		file = "";
		if (infoClient.req.t_result.target == "/home" || infoClient.req.t_result.target == "/")
			file = cwdPath + "/resource/static/index.html";
		else if (infoClient.req.t_result.target == "/server")
			file = cwdPath + "/resource/static/server.html";
		else if (infoClient.req.t_result.target == "/submit")
			file = cwdPath + "/resource/static/submit.html";
		else if (infoClient.req.t_result.target == "/upload")
			file = cwdPath + "/resource/static/upload.html";
		else if (infoClient.req.t_result.target == "/delete")
			file = cwdPath + "/resource/static/delete.html";
	}
	else if (infoClient.req.t_result.method == POST)
	{
		file = "";
		if (infoClient.req.t_result.target == "/www/cgi-bin/submit.py")
			file = cwdPath + "/submit_out.html";
		else if (infoClient.req.t_result.target == "/www/cgi-bin/upload.py")
			file = cwdPath + "/upload_out.html";
	}

	std::cout << "\n\nresponse file : " << file << "  \n\n";
	stat(file.c_str(), &st);
	contentLen = st.st_size;
	setContentLength(contentLen);
	// std::cout << "\n\n============\n" << infoClient.req.t_result.contentLen << "\n===========\n\n";

	header << getHttpVersion() << " " << getStatusCode() << " " << getStatusMsg() << CRLF;
	header << "Content-Type: " << getContentType() << "; charset=utf-8" << CRLF;
	header << "Connection: " << getConnection() << CRLF;
	header << "Date: " << timeStamp() << CRLF;
	header << "Server: "
		   << "little webserver" << CRLF;
	// header << "Transfer-Encoding : chunked" << CRLF;
	header << "Content-Length: " << getContentLength() << CRLF;

	return (header.str());
}

std::string
Response::resMsgBody(std::string srcLocation)
{
	std::stringstream body;
	_fileFd = open(srcLocation.c_str(), O_RDONLY);
	if (_fileFd == -1)
		std::cerr << "	ERROR: file open error\n";
	int valRead = read(_fileFd, _fileBuff, sizeof(_fileBuff));
	if (valRead == -1)
		std::cerr << "	ERROR: read\n";
	std::cerr << "srcLocation : " << srcLocation << "\n";
	std::cerr << " valRead : " << valRead << " fileFd : " << _fileFd << "\n";
	if (_fileFd == -1 || valRead == -1)
	{
		body << "<h1>ERROR</h1><img src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAUAAAAFCAYAAACNbyblAAAAHElEQVQI12P4//8/w38GIAXDIBKE0DHxgljNBAAO9TXL0Y4OHwAAAABJRU5ErkJggg==\" alt=\"Red dot\" />";
	}
	else
		body << _fileBuff;

	return body.str();
}
