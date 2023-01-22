#include "Response.hpp"

int
Response::openResponse()
{
	std::string cwdPath = this->getCwdPath();
	std::string srcPath = "";
	std::string target = p_infoClient->reqParser.t_result.target;

	int isFile = isValidTarget(target);
	int status = 0;
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
			// fd = open("500error.html", O_RDONLY);
		break;

	case POST:
		this->PostCase(target);
		break;

	case DELETE:
		this->DeleteCase(target);
		break;
	}
	return (fd);
}

int
Response::isValidTarget(std::string &target)
{
	std::string srcPath;
	if (target == "/" || target == "/home")
		target = "index.html";
	else if (target == "/submit")
		target = "submit.html";
	else if (target == "/upload")
		target = "upload.html";
	else if (target == "/server")
		target = "server.html";
	else if (target == "/post.py")
		target = "post.py";
	else if (target == "/upload.py")
		target = "delete.py";
	else if (target == "/submit.py")
		target = "submit.py";

	if (p_infoClient->reqParser.t_result.method == GET)
		srcPath = this->getCwdPath() + "/www/statics";
	if (p_infoClient->reqParser.t_result.method == POST)
		srcPath = this->getCwdPath() + "/www/cgi-bin";

	std::cout << "path : " << srcPath << std::endl;
	DIR *dir = opendir(srcPath.c_str());
	struct dirent *dirent = NULL;
	// 405 etc to be added.
	while (true)
	{
		dirent = readdir(dir);
		if (!dirent)
			break;
		if (strcmp(dirent->d_name, (target).c_str()) == SUCCESS)
		{
			(target).insert(0, "/");
			struct stat ss;
			std::string srcPath = srcPath + target;
			if (stat(srcPath.c_str(), &ss) == -1 || S_ISREG(ss.st_mode) != true)
				return (500);
			return (200);
		}
	}
	return (404);
}


int
Response::GetCase(std::string &target)
{
	std::string srcPath = getCwdPath() + "/www/statics" + target;
	// p_infoClient->status = Res::Making;
	int fd = -1;

	std::cout << "srcPath : " << srcPath << std::endl;
	if ((fd = open(srcPath.c_str(), O_RDONLY)) == -1)
		return (fd);
}

int
Response::PostCase(std::string &target)
{
	if (isCgiIng == true)
		return ;
	std::string cwdPath = this->getCwdPath();
	std::string execPath = getCwdPath() + "/www/cgi-bin" + target;
	char const *args[2] = {execPath.c_str(), NULL};

	CGI cgi;
	cgi.initEnvMap(*p_infoClient);
	cgi.envMap.insert(std::pair<std::string, std::string>("UPLOAD_PATH", cwdPath + "/uploaded/"));
	cgi.envMap.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", args[0]));

	char *cgiEnv[cgi.envMap.size() + 1];
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
		waitpid(pid, NULL, WNOHANG);
		p_infoClient->m_responser->isCgiIng = true;
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

int
Response::DeleteCase(std::string &target)
{

}
