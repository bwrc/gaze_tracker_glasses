#ifndef TIMING_H
#define TIMING_H


#ifdef WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <stdlib.h>


namespace utils {


    class Timing {

    public:

        Timing() {

            markTime();

        }

        void markTime() {

#ifdef WIN32
            
            _ftime(&t1);
#else
            gettimeofday(&t1, NULL);
#endif
        }

        long getElapsedMicros() {

#ifdef WIN32
            
            _ftime(&t2);

            return (t2.time - t1.time) * 1000000L + (t2.millitm - t1.millitm) * 1000L;
#else

            gettimeofday(&t2, NULL);
            return (t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);
      
#endif

        }

    private:

#ifdef WIN32
        struct _timeb t1, t2;
#else
        struct timeval t1, t2;
#endif



    };


} // end of "namespace utils"


#endif

