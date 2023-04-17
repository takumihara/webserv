#ifndef CLIENT_SOCKET_HPP_
#define CLIENT_SOCKET_HPP_

//#include "AbstractSocket.hpp"

class ClientSocket {
	private:
		int state_;
		int fd_;


	public:
		enum class SocketState {
			kSocFree,
			kSocReading,
			kSocWriting,
		};
		ClientSocket(int fd, SocketState state);
		~ClientSocket();
		void	notify();
	

};

#endif