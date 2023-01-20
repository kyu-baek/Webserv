#ifndef DEFINE_HPP
#define DEFINE_HPP

#define SUCCESS 0
#define FAIL -1
#define BUFFER_SIZE 10

#define MAX_NUM_EVENTS 1024

#define NONE -1

#define TIMER 60

enum {
	GET,
	POST,
	DELETE
};

enum
{
	FileError = -1,
	FileNone = 0,
	FileMaking = 1,
	FileComplete = 2
};

enum
{
	ResError = -1,
	ResNone = 0,
	ResMaking = 1,
	ResComplete = 2
};

enum
{
	SendError = -1,
	SendNone = 0,
	SendMaking = 1,
	SendComplete = 2
};

#endif
