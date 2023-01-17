#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../../includes/Define.hpp"
#include "../../includes/libraries.hpp"
#include "../../InfoFd.hpp"
#include "ResponseInfo.hpp"
class InfoClient;

class Response : public ResponseInfo
{
	public:
		InfoClient *infoClient;

	public:
		void makeResponse();

};

#endif
