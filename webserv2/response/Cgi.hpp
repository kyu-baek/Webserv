#ifndef CGI_HPP
#define CGI_HPP

#include "../includes/Define.hpp"
#include "../includes/cppLibrary.hpp"
#include "../connection/InfoClient.hpp"

class InfoClient;

class CGI {
	public:
		std::map<std::string, std::string> envMap;

	public:
		void initEnvMap(InfoClient &infoClient)
		{
			envMap.insert(std::pair<std::string, std::string>("AUTH_TYPE", ""));
			envMap.insert(std::pair<std::string, std::string>("GATE_INTERFACE", "CGI/1.1"));
			envMap.insert(std::pair<std::string, std::string>("SERVER_NAME", "webserver"));
			envMap.insert(std::pair<std::string, std::string>("SERVER_PROTOCOL", "HTTP/1.1"));
			envMap.insert(std::pair<std::string, std::string>("REMOTE_USER", ""));
			envMap.insert(std::pair<std::string, std::string>("CONTENT_LENGTH", "-1"));
			envMap.insert(std::pair<std::string, std::string>("CONTENT_TYPE", ""));
			if (infoClient.req.t_result.method == GET)
				envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "GET"));
			else if (infoClient.req.t_result.method == POST)
				envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "POST"));
			else if (infoClient.req.t_result.method == DELETE)
				envMap.insert(std::pair<std::string, std::string>("REQUEST_METHOD", "DELETE"));
			envMap.insert(std::pair<std::string, std::string>("QUERY_STRING", infoClient.req.t_result.query ));
			envMap.insert(std::pair<std::string, std::string>("SERVER_PORT", std::to_string(infoClient._server->_port)));
		}

};

#endif



// void	Cgi::set_env_map(void *ptr_void)
// {
// 	/*
// 		Set cgi body to empty string to check when we execve to know if there is something to send in STDIN
// 	*/
// 	_cgi_body = "";

// 	Request *ptr_request = (Request *)ptr_void;
// 	_env_map["AUTH_TYPE"] = "";
// 	_env_map["CONTENT_LENGTH"] = "0";
// 	_env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
// 	_env_map["PATH_TRANSLATED"] = "";//_args[0];
// 	_env_map["REDIRECT_STATUS"] = "200";
// 	_env_map["REQUEST_METHOD"] = ptr_request->header["method"];
// 	_env_map["SCRIPT_FILENAME"] = _args[1];
// 	_env_map["SCRIPT_PORT"] = ptr_request->header["port"];
// 	_env_map["SERVER_NAME"] = "webserv";
// 	_env_map["SERVER_PROTOCOL"] = "HTTP/1.1";

// 	if (ptr_request->header["method"] == "GET" && ptr_request->header["args"] != "")
// 	{
// 		_env_map["QUERY_STRING"] = ptr_request->header["args"];
// 		_cgi_body = ptr_request->header["args"];
// 	}
// 	if (ptr_request->header["method"] == "POST")
// 	{
// 		_env_map["CONTENT_LENGTH"] = ptr_request->header["CONTENT-LENGTH"];
// 		_env_map["CONTENT_TYPE"] = ptr_request->header["CONTENT-TYPE"];
// 		_cgi_body = ptr_request->header["body"];
// 	}
// }

/*
AUTH_TYPE
CONTENT_LENGTH
CONTENT_TYPE
GATEWAY_INTERFACE
PATH_INFO
PATH_TRANSLATED
QUERY_STRING
REMOTE_ADDR
REMOTE_IDENT
REMOTE_USER
REQUEST_METHOD
REQUEST_URI
SCRIPT_NAME
SERVER_NAME
SERVER_PORT
SERVER_PROTOCOL
SERVER_SOFTWARE
*/
