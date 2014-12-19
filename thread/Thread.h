#ifndef THREAD_H
#define THREAD_H


#include <pthread.h>



class Thread {

public:

    Thread();
    virtual ~Thread();


    virtual bool start();
    virtual void end();

    /*
     * Returns the state of this thread
     */
    bool isRunning();

    virtual void run() = 0;

protected:

    void sleepMs(int ms);
    void killSelf();

private:

    /*
     * Has start been executed successfully?
     */
    volatile bool mbInitOk;

    pthread_t thread;


    pthread_mutex_t mutexRunning;

    pthread_mutex_t mutexSleep;
    pthread_cond_t condSleep;

    volatile bool bRunning;


};



#endif

