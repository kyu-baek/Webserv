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
		Response() : ResponseInfo(), m_totalBytes(0), m_sentBytes(0) {}

	public:
		InfoClient *m_infoClientPtr;
		FileManage *m_fileManagerPtr;
		std::string m_resMsg;
		size_t m_totalBytes;
		size_t m_sentBytes;
		std::string cgiOutPath;
		std::string cgiOutTarget;

	public:
		void openResponse();
		void initResponse();
		void startResponse();
	
	public:
		int sendResponse();
		size_t changePosition(int n);
		size_t getSendResultSize() const;
		const char * getSendResult() const;
		void clearResponseByte();

};

#endif
