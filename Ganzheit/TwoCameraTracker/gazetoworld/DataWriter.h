#ifndef DATA_WRITER_H
#define DATA_WRITER_H

#include "Thread.h"


class DataWriter : public Thread {

public:


    DataWriter() : Thread() {}
    virtual ~DataWriter() {}

    virtual bool init(const std::string &) = 0;
    virtual bool addFrames(const CameraFrame *_f1, const CameraFrame *_f2) = 0;
    virtual bool addResults(const std::vector<char> &) = 0;
    virtual int getBufferState() = 0;

};


#endif

