#include "Timer.h"


MyTimer::MyTimer() {
	gettimeofday(&t1, 0);
}


void MyTimer::markTime() {
	gettimeofday(&t1, 0);
}


unsigned long MyTimer::getElapsed_millis() {
	gettimeofday(&t2, 0);
	long sec	= t2.tv_sec - t1.tv_sec;
	long micros	= t2.tv_usec - t1.tv_usec;
	unsigned long millis = (unsigned long)(1000.0 * sec + micros / 1000.0 + 0.5);

	return millis;
}


unsigned long MyTimer::getElapsed_micros() {
	gettimeofday(&t2, 0);

	long sec				= t2.tv_sec - t1.tv_sec;
	long micros_tmp			= t2.tv_usec - t1.tv_usec;
	unsigned long micros	= (unsigned long)(1000000.0 * sec + micros_tmp);

	return micros;
}
