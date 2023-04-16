#ifndef ABSTRACT_SOCKET_HPP_
#define ABSTRACT_SOCKET_HPP_

class AbstractSocket {
	private:
		int fd;

	public:
		virtual void	makeSocket() = 0;
};

#endif