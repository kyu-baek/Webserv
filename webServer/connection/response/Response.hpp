#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../../includes/Define.hpp"
#include "../../includes/libraries.hpp"
#include "../../InfoFd.hpp"
#include "ResponseInfo.hpp"
#include "CGI.hpp"

class InfoClient;

class Response : public ResponseInfo
{
	public:
		InfoClient *p_infoClient;

	public:
		int fds[2];
		bool isCgiIng;

	public:
		int openResponse();
		int  isValidTarget(std::string &target);

	public:
		int GetCase(std::string &target);
		int PostCase(std::string &target);
		int DeleteCase(std::string &target);

};

#endif
