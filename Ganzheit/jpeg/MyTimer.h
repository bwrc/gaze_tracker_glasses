#include <sys/time.h>

class MyTimer {
	public:
		MyTimer();

		void markTime();
		unsigned long getElapsed_millis();
		unsigned long getElapsed_micros();
	private:
		struct timeval t1, t2;
};

