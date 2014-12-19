#ifndef VIDEO_SYNC_H
#define VIDEO_SYNC_H


#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Thread.h"
#include "DualFrameReceiver.h"
#include "SimpleCapture.h"


class VideoSync : public Thread {

public:

    VideoSync();

    /* Inherited from Thread */
    void run();


    /* Mimics the behavior of VideoHandler::init() */
    bool init(const std::vector<VideoInfo> &info, DualFrameReceiver *_r);


private:

    DualFrameReceiver *frameReceiver;

	cv::VideoCapture capEye;
	cv::VideoCapture capScene;

    SimpleCapture simpleCapEye;
    SimpleCapture simpleCapScene;

    /*
     * Tells if this is a video file or a camera device
     */
    bool m_bVideoFile;

    /*
     * Frames per second. Used for sleeping only if video files are used.
     */
    int m_nFPS;


};




#endif
