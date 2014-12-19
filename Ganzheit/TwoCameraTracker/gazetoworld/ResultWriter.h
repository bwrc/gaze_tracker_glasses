#ifndef RESULT_WRITER_H
#define RESULT_WRITER_H


#include <list>
#include <vector>
#include <string>
#include <pthread.h>
#include <fstream>
#include "CameraFrame.h"
#include "BinaryResultParser.h"
#include <sys/time.h>
#include "DataWriter.h"


class QueueData {

public:

    QueueData() {
        m_pData = NULL;
        m_nSz = 0;
    }


    QueueData(char *data, int nSz) {
        m_pData = data;
        m_nSz = nSz;
    }

    void release() {
        delete[] m_pData;
        m_pData = NULL;
        m_nSz = 0;
    }

    bool empty() {
        return (bool)(m_pData == NULL || m_nSz == 0);
    }

    char *m_pData;
    int m_nSz;


};



class ResultWriter : public DataWriter {

public:

    ResultWriter();
    ~ResultWriter();


    /*
     * Inherited from Thread
     */
    void run();


    /*
     * Inherited from DataWriter
     */

    /*
     * parentDir must contain the trailing '/'.
     */
    bool init(const std::string &parentDir);
    bool addFrames(const CameraFrame *_f1, const CameraFrame *_f2);
    bool addResults(const std::vector<char> &);
    int getBufferState();

private:

    void waitForData();

    QueueData getElement();

    void write(const QueueData &el);

    /* Output files */
    std::ofstream streamResults;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    std::list<QueueData> queue;


    std::string workingDir;

};


#endif

