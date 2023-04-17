#ifndef CLIENT_SOCKET_HPP_
#define CLIENT_SOCKET_HPP_

//#include "AbstractSocket.hpp"

class ClientSocket {
	public:
		enum class SocketState {
			kSocFree,
			kSocReading,
			kSocWriting,
		};
		ClientSocket(int fd, SocketState state);
		~ClientSocket();
		void	notify();
		void	handle_request();
	private:
		SocketState state_;
		int fd_;

};

#endif


ClientSocket::ClientSocket(int fd, SocketState state): state_(state), fd_(fd) {}

void	ClientSocket::notify() {
	return ;
}

