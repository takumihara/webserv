#ifndef ABSTRACT_SOCKET_HPP_
#define ABSTRACT_SOCKET_HPP_

class EventManager;

class AbstractSocket {
	public:
		AbstractSocket(int fd) : fd_(fd) {}
		virtual ~AbstractSocket() {}
		virtual	void	notify(EventManager &event_manager) = 0;

	protected:
		int fd_;
};

#endif
