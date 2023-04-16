#include <fcntl.h>
#include <unistd.h>
int main () {
	int fd = open("test.data", O_CREAT | O_RDWR);
	for (int i = 0; i < 10000000; i++) {
		write(fd, "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", 100);
	}
}