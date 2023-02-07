#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "Config_struct.hpp"
#include "BaseServer.hpp"


/*	Request class
	author	:kbaek
	date 	:2022.12.10

*/
class BaseServer;

struct Config_base
{
	Config_base() { numOfServer = 0;}
	std::vector<BaseServer> getConfigBase();
	int	getNumOfServer();
	void print_config();

protected:
	std::vector<BaseServer> 		base;
	int								numOfServer;
};

class Config : public Config_base
{
public:
	Config();
	~Config();
	Config(const std::string path);

	class	FileNotFoundException: public std::exception{
		virtual const char	*what() const throw();
	};

private:
	void configInit(const std::string path);
	void configParse();
	void serverInit(int, int);
	void checkVaildServers();
	
private:
	std::vector<std::string> file;
};

std::ostream &operator<<(std::ostream &ost, const Config &conf);


#endif
