#include "Thread.h"
#include <stdio.h>

#ifdef WIN32
#include <sys/timeb.h>
#endif


static void *thread_fnct(void *arg) {

	Thread *_this = (Thread *)arg;
	_this->run();

	pthread_exit(NULL);

	return NULL;

}



Thread::Thread() {

    pthread_mutex_init(&mutexRunning, NULL);

    pthread_mutex_init(&mutexSleep, NULL);
    pthread_cond_init(&condSleep, NULL);

    bRunning = false;

    // false until the thread has been created successfully
    mbInitOk = false;

}


Thread::~Thread() {

    pthread_mutex_destroy(&mutexRunning);

    pthread_mutex_destroy(&mutexSleep);
    pthread_cond_destroy(&condSleep);

}


bool Thread::start() {

    bRunning = true;
    mbInitOk = true;

	// thread attributes
	pthread_attr_t attr;

	/*
	 * initialize and set thread detached attribute.
	 * On some systems the thread will not be created
	 * as joinable, so do it explicitly. It is more
	 * portable this way
	 */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// create the thread
	int success = pthread_create(&thread, &attr, &thread_fnct, (void *)this);
	// destroy the attributes
	pthread_attr_destroy(&attr);

	// was the thread creation successfull...
	if(success != 0) {

        printf("Thread::start() FAILED\n");

        bRunning = false;
        mbInitOk = false;

		// ...nope...

		return false;
	}


	// ...yes it was ...

	return true;

}


void Thread::killSelf() {

    pthread_mutex_lock(&mutexRunning);

    bRunning = false;

    pthread_mutex_unlock(&mutexRunning);

}


void Thread::end() {

    pthread_mutex_lock(&mutexRunning);

    bRunning = false;

    pthread_mutex_unlock(&mutexRunning);

    if(mbInitOk) {
        pthread_join(thread, NULL);
    }

}


bool Thread::isRunning() {

    pthread_mutex_lock(&mutexRunning);

    bool retVal = bRunning;

    pthread_mutex_unlock(&mutexRunning);


    return retVal;

}


void Thread::sleepMs(int millis) {

    // get the system time
    struct timespec ts;

    static const long BILLION = 1e9;
    static const long MILLION = 1e6;

#ifdef WIN32

    struct _timeb currSysTime;
    _ftime(&currSysTime);
    ts.tv_sec = static_cast<long>(currSysTime.time + millis / 1000);
    ts.tv_nsec = currSysTime.millitm * MILLION + (millis % 1000) * MILLION; // millis * 1e6 => nanosecond

#else

    clock_gettime(CLOCK_REALTIME, &ts);

    // add the requested wait time
    ts.tv_sec  += millis / 1000;
    ts.tv_nsec += (millis % 1000) * MILLION; // millis * 1e6 => nanosecond

#endif


    // account for overflow
    if(ts.tv_nsec >= BILLION) {
        ++ts.tv_sec;
        ts.tv_nsec -= BILLION;
    }


    /*****************************************************************
     * Wait on the condition
     *****************************************************************/
    pthread_mutex_lock(&mutexSleep);

    // on success returns zero
    /*int ret =*/ pthread_cond_timedwait(&condSleep, &mutexSleep, &ts);

    pthread_mutex_unlock(&mutexSleep);

}

