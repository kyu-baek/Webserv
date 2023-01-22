#ifndef DEFINE_HPP
#define DEFINE_HPP

#define SUCCESS 0
#define FAIL -1
#define BUFFER_SIZE 1024

#define MAX_NUM_EVENTS 1024

#define NONE -1

#define TIMER 60

#define CHILD 0

enum {
	GET,
	POST,
	DELETE
};

namespace Client
{
	enum
	{
		Error = -1,
		None = 0,
		Making = 1,
		Complete = 2
	};
}

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


#endif
