/***************************************************/
/* CODED BY JIN H. BANG ===========================*/
/***************************************************/

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../includes/Define.hpp"
#include "../connection/InfoServer.hpp"
#include "../connection/InfoClient.hpp"
#include "HttpResInfo.hpp"

class Response : public HttpResInfo
{
	public:
		void responseToClient(int clientSocket, InfoClient infoClient);
		std::string makeResponseGET(InfoClient &infoClient);
		std::string makeResponseERR();

	private:
		std::string resMsgBody(std::string srcLocation);
		std::string resMsgHeader(InfoClient &infoClient);
		// std::string httpRes2XX();
		// std::string httpRes3XX();
		// std::string httpRes4XX();
		// std::string httpRes500();
	private:
		char _fileBuff[1024];
		int _fileFd;
};



#endif



/*
void
Account::_displayTimestamp( void )
{
	std::time_t timeBuff = std::time(NULL);
	struct tm	*time = std::localtime(&timeBuff);

	std::cout << "[" << 1900 + time->tm_year << 1 + time->tm_mon << time->tm_mday
		<< "_" << time->tm_hour << time->tm_min << time->tm_sec << "] ";
}

*/
