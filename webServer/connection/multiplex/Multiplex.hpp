#ifndef MULTIPLEX_HPP
#define MULTIPLEX_HPP

#include "../../includes/libraries.hpp"
#include "../../includes/Define.hpp"

class Multiplex
{
	public:
		int m_kq;
		std::vector<struct kevent> m_changeList;
		struct kevent m_eventList[MAX_NUM_EVENTS];

	public:
		struct kevent *currEvent;

	public:
		Multiplex()
		: currEvent(NULL) {}
		void declareKqueue();
		void enrollEventToChangeList(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);
		int senseEvents();
		void clearChangeList();
		struct kevent const *getEventList() const;
};

#endif
