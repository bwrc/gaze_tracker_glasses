#ifndef DEV_NULL_WRITER_H
#define DEV_NULL_WRITER_H


#include "DataWriter.h"


class DevNullWriter : public DataWriter {

public:

    DevNullWriter() : DataWriter() {}

    ~DevNullWriter() {}

    bool init(const std::string &) {return true;}

    bool addFrames(const CameraFrame *_f1, const CameraFrame *_f2) {return true;}

    bool addResults(const std::vector<char> &) {return true;}

    void run() {}

    void end() {}

    int getBufferState() {return 0;}

};


#endif

