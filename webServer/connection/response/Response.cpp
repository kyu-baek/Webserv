#include "Response.hpp"

void
Response::openResponse()
{
	std::string cwdPath = this->getCwdPath();
	std::string srcPath = "";

	int isFile = isValidTarget(p_infoClient->reqParser.t_result.target);

	if (isFile == -1)
		return ;

	// if (p_infoClient->reqParser.t_result.method == GET)
	// {
	// 	std::cerr << "GET RESPONSE\n";
	// 	std::cerr << "isFile :" << isFile << "\n";
	// 	if (isFile == true)
	// 	{
	// 		srcPath = cwdPath + "/www/statics" + p_infoClient->reqParser.t_result.target;
	// 		p_infoClient->status = Res::Making;
	// 		int fd = -1;
	// 		struct stat ss;
	// 		std::cout << "srcPath : "<<srcPath << std::endl;
	// 		if (stat(srcPath.c_str(), &ss) == -1 || S_ISREG(ss.st_mode) != true || (fd = open(srcPath.c_str(), O_RDONLY)) == -1)
	// 			isFile = 500;
	// 		else
	// 		{
	// 			m_fileManagerPtr->m_file.fd = fd;
	// 			m_fileManagerPtr->m_infoFileptr = new InfoFile(); // to be deleted
	// 			m_fileManagerPtr->m_infoFileptr->p_infoClient = p_infoClient;
	// 			m_fileManagerPtr->m_infoFileptr->srcPath = srcPath;
	// 			p_infoClient->status = Res::Making; // added
	// 		}
	// 	}
	// 	if (isFile == 404 || isFile == 500)
	// 	{
	// 		std::cerr << "	NO FILE FOUND\n";
	// 		//404 response
	// 	}
	// }

	// if (p_infoClient->reqParser.t_result.method == POST)
	// {

	// }
}

int
Response::isValidTarget(std::string &target)
{
	if (target == "/" || target == "/home")
		target = "index.html";
	else if (target == "/submit")
		target = "submit.html";
	else if (target == "/upload")
		target = "upload.html";
	else if (target == "/server")
		target = "server.html";

	std::string staticPath = this->getCwdPath() + "/www/statics";
	std::cout << "path : " << staticPath << std::endl;
	DIR *dir = opendir(staticPath.c_str());
	struct dirent *dirent = NULL;
	while (true)
	{
		dirent = readdir(dir);
		if (!dirent)
			break;
		if (strcmp(dirent->d_name, (target).c_str()) == SUCCESS)
		{
			(target).insert(0, "/");
			return (1);
		}
	}
	return (404);
}
