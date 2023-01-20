#include "FileManage.hpp"

bool
FileManage::isValidStaticSrc(std::string *target)
{
	if (*target == "/")
		*target = "index.html";
	std::string staticPath = this->getCwdPath() + "/www/statics";
	std::cout << "path : " << staticPath << std::endl;
	DIR *dir = opendir(staticPath.c_str());
	struct dirent *dirent = NULL;
	while (true)
	{
		dirent = readdir(dir);
		if (!dirent)
			break;
		if (strcmp(dirent->d_name, (*target).c_str()) == SUCCESS)
		{
			(*target).insert(0, "/");
			return (true);
		}
	}
	return (false);
}

int
FileManage::readFile(int fd)
{
	char buffer[BUFFER_SIZE + 1];

	memset(buffer, 0, sizeof(buffer));
	//std::cout << "reading\n";
	ssize_t size = read(fd, buffer, sizeof(buffer));
	if (size < 0)
	{
		close(fd);
		m_infoFileptr->m_fileFdMapPtr->erase(fd);
		m_file.buffer.clear();
		return FileError;
	}
	m_file.buffer += std::string(buffer, size);
	m_file.size += size;
	if (size < BUFFER_SIZE)
	{
		// close(fd);
		// _fdMap.erase(fd);
		return FileComplete;
	}
	return FileMaking;
}
