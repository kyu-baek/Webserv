#include "Multiplex.hpp"

void
Multiplex::declareKqueue()
{
	m_kq = kqueue();
	if (m_kq == -1)
	{
		std::cerr << "kqueue() error : ";
		exit(1); // make it throw later
	}
}

void
Multiplex::enrollEventToChangeList(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
	struct kevent event;

	// EV_SET(&event, ident, filter, flags, fflags, data, udata); //ident = socketFd
	EV_SET(&event, ident, filter, flags, fflags, data, udata);
	m_changeList.push_back(event);
}

int
Multiplex::senseEvents()
{
	int sensedEvents;

	sensedEvents = kevent(m_kq, &m_changeList[0], m_changeList.size(), m_eventList, MAX_NUM_EVENTS, NULL); // SENSE NEW_EVENTS(연결 감지)
	if (sensedEvents == -1)
	{
		std::cerr << "kevent() error :";
		exit(1); // make it throw later
	}
	return (sensedEvents);
}

void
Multiplex::clearChangeList()
{
	m_changeList.clear();
}

struct kevent const *
Multiplex::getEventList() const
{
	return (m_eventList);
}
