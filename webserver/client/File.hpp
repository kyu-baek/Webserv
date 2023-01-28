#ifndef FLIE_HPP
# define FLIE_HPP
#include "../includes/libraries.hpp"

struct FileEvent
{
	int fd;
	std::size_t size;
	std::string buffer;
	std::size_t m_totalBytes;
	std::size_t m_sentBytes;
	std::size_t m_pipe_sentBytes;
	int inFds[2];
	int outFds[2];
	int isFile;
	std::string srcPath;
	FileEvent() : fd(-1), size(0), buffer(""), m_totalBytes(0), m_sentBytes(0), m_pipe_sentBytes(0){}
};

#endif
