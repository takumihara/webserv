#include "EventManager.hpp"
#include "debug.hpp"

int main() {
	EventManager mg;
	DEBUG_PUTS("hello\n");
	mg.eventLoop();
	return 1;
}