#ifndef RESULT_STREAMER_H
#define RESULT_STREAMER_H


#include <string>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "BinaryResultParser.h"
#include <fstream>



class DataStream {

public:

    virtual bool init(const std::string &folder, const std::string &_fileName);


protected:

    std::string getNextFileName();

private:



    std::string fileName;

    /* partX, X is the folder id */
    int folderID;

    std::string parentDir;

};


class VideoStream : public DataStream {

public:

    bool init(const std::string &folder, const std::string &_fileName);

    bool getNextFrame(cv::Mat &frame);

	cv::VideoCapture cap;

};


class ResultStream : public DataStream {

public:

    bool init(const std::string &folder, const std::string &_filename);

    bool getNextDataPacket(ResultData &data);

private:

    std::ifstream resIn;

};


class ResultStreamer {

    public:

    enum StreamError {

        STREAM_OK,
        STREAM_FINISHED,
        STREAM_ERROR

    };


    ResultStreamer();

    bool reset();

    bool init(const std::string &folder);

    int get(cv::Mat &imgEye, cv::Mat &imgScene, ResultData &data);


    static bool exists(const std::string &dir);

    int getFourCC();
    int getFps();

    cv::Size getDim();


private:

    VideoStream sceneStream;
    VideoStream eyeStream;

    ResultStream resStream;

    std::string parentDir;

    /* Used for syncing the results to the frames */
    unsigned int framePos;

    /* Used for syncing the results to the frames */
    ResultData prevDataPacket;


};



#endif
