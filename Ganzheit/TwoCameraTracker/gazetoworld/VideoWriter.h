#ifndef VIDEOWRITER_H
#define VIDEOWRITER_H


#include <list>
#include <vector>
#include <string>
#include <pthread.h>
#include <fstream>
#include "CameraFrame.h"
#include "BinaryResultParser.h"
#include <sys/time.h>
#include "DataWriter.h"


class QueueElement {

public:

    enum ElementType {

        TYPE_FRAME1,
        TYPE_FRAME2,
        TYPE_RESULTS,
        TYPE_NO_TYPE

    };


    QueueElement(char *_data = NULL, int _size = 0, int _type = TYPE_NO_TYPE) {
        data = _data;
        size = _size;
        type = _type;
    }

    void release() {

        delete[] data;

        data = NULL;
        size = 0;
        type = TYPE_NO_TYPE;

    }

    bool empty() {return data == NULL || size == 0;}

    char *data;
    int size;
    int type;

};



class VideoWriter : public DataWriter {

public:

    VideoWriter();
    ~VideoWriter();


    /*
     * parentDir must contain the trailing '/'.
     */
    bool init(const std::string &parentDir);

    bool addFrames(const CameraFrame *_f1, const CameraFrame *_f2);

    bool addResults(const std::vector<char> &);

    void run();

    int getBufferState();

private:

    void waitForData();

    long elapsedSeconds();
    void zeroTimer();

    bool createFolder(const std::string &oputDir);
    bool createNewFiles();
    void setTerminated();

    QueueElement getElement();

    void write(const QueueElement &el);

    bool wait(int millis);

    /* Output files */
    std::ofstream streamEyeCam;
    std::ofstream streamSceneCam;
    std::ofstream streamResults;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    std::list<QueueElement> queue;

    struct timeval timeStart;

    std::string workingDir;

    /* How many backups */
    int countBackup;

};


#endif

