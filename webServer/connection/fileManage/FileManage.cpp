#include "FileManage.hpp"

int
FileManage::isValidStaticSrc(std::string &target)
{
	if (target == "/" || target == "/home")
		target = "index.html";
	else if (target == "/submit")
		target = "submit.html";
	else if (target == "/upload")
		target = "upload.html";
	else if (target == "/server")
		target = "server.html";
	else if (target == "/favicon.ico")
		return (-1);

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

int
FileManage::readFile(int fd)
{
	char buffer[BUFFER_SIZE + 1];

	memset(buffer, 0, sizeof(buffer));
	//std::cout << "reading\n";
	ssize_t size = read(fd, buffer, BUFFER_SIZE);
	//std::cout << size << std::endl;
	if (size < 0)
	{
		close(fd);
		m_infoFileptr->m_fileFdMapPtr->erase(fd);
		m_file.buffer.clear();
		return File::Error;
	}
	m_file.buffer += std::string(buffer, size);
	m_file.size += size;
	if (size < BUFFER_SIZE)
	{
		// close(fd);
		// _fdMap.erase(fd);
		return File::Complete;
	}
	return File::Making;
}

void
FileManage::clearFileEvent()
{
	m_file.fd = -1;
	m_file.size = 0;
	m_file.buffer = "";
}
