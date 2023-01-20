#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../../includes/Define.hpp"
#include "../../includes/libraries.hpp"
#include "../../InfoFd.hpp"
#include "../fileManage/FileManage.hpp"
#include "ResponseInfo.hpp"
class InfoClient;
class FileManage;

class Response : public ResponseInfo
{
	public:
		InfoClient *m_infoClientPtr;
		FileManage *m_fileManagerPtr;
		std::string m_resMsg;
		size_t m_totalBytes;


	public:
		void openResponse();
		void initResponse();
		void startResponse();
};

#endif
