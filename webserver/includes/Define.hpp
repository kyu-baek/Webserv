#ifndef DEFINE_HPP
#define DEFINE_HPP

#define SUCCESS 0
#define FAIL -1
#define BUFFER_SIZE 4096

#define MAX_NUM_EVENTS 1024

#define NONE -1
#define AUTO -300
#define INDEX -400
#define REDIRECTION -500
#define TIMER 60

#define RED "\x1b[0;31m"
#define GREEN "\x1b[0;32m"
#define YELLOW "\x1b[0;33m"
#define BLUE "\x1b[0;34m"
#define MAGENTA "\x1b[0;35m"
#define RESET "\x1b[0m"

enum {
	GET,
	POST,
	DELETE
};

namespace File
{
	enum
	{
		Error = -1,
		None = 0,
		Making = 1,
		Complete = 2
	};
}

namespace Res
{
	enum
	{
		Error = -1,
		None = 0,
		Making = 1,
		Complete = 2
	};
}

namespace Send
{
	enum
	{
		Error = -1,
		None = 0,
		Making = 1,
		Complete = 2
	};
}

namespace Write
{
	enum
	{
		Error = -1,
		None = 0,
		Making = 1,
		Complete = 2
	};
}

#endif
