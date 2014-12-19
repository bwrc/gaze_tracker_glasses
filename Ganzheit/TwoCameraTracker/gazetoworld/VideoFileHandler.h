#ifndef VIDEO_FILE_HANDLER_H
#define VIDEO_FILE_HANDLER_H


#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Thread.h"
#include "DualFrameReceiver.h"


class VideoFileHandler : public Thread {

public:

    VideoFileHandler();
    ~VideoFileHandler();

    /* Inherited from Thread */
    void run();


    /* Mimics the behavior of VideoHandler::init() */
    bool init(const std::vector<VideoInfo> &info, DualFrameReceiver *_r);


private:

    DualFrameReceiver *frameReceiver;

	cv::VideoCapture capEye;
	cv::VideoCapture capScene;


};




#endif
