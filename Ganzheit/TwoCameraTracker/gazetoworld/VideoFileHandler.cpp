#include "VideoFileHandler.h"




VideoFileHandler::VideoFileHandler() {


}


VideoFileHandler::~VideoFileHandler() {

}


bool VideoFileHandler::init(const std::vector<VideoInfo> &info, DualFrameReceiver *_r) {

    if(info.size() != 2) {
        return false;
    }

    frameReceiver = _r;

    capEye.open(info[0].devname);
    if(!capEye.isOpened()) {
        return false;
    }

    capScene.open(info[1].devname);
    if(!capScene.isOpened()) {
        return false;
    }

    return true;

}


void VideoFileHandler::run() {

    while(isRunning()) {

        cv::Mat imgEye, imgScene;

        capEye >> imgEye;
        capScene >> imgScene;

        if(imgEye.empty() || imgScene.empty()) {
            continue;
        }

        int w = imgEye.cols;
        int h = imgEye.rows;
        int bpp = 3;

        // create a header for the data. Does not copy data.
        CameraFrame frameEye(w,
                             h,
                             bpp,         // bytes per pixel
                             imgEye.data,
                             w*h*bpp,
                             FORMAT_BGR,
                             false,       // do not copy data
                             false);      // do not become parent, i.e. do not destroy data in destructor


        // create a header for the data. Does not copy data.
        CameraFrame frameScene(w,
                               h,
                               bpp,         // bytes per pixel
                               imgScene.data,
                               w*h*bpp,
                               FORMAT_BGR,
                               false,       // do not copy data
                               false);      // do not become parent, i.e. do not destroy data in destructor


        frameReceiver->frameReceived(&frameEye,   0);
        frameReceiver->frameReceived(&frameScene, 1);


        size_t states[2];
		frameReceiver->getWorkerBufferStates(states);

		/* Return the maximum allowed worker buffer queue size */
		int maxBuffSz = frameReceiver->getMaxBufferSize();

        int max = std::max(states[0], states[1]);

        if(max >= maxBuffSz) {
            sleepMs(1000);
        }

        sleepMs(30);

    }

}

