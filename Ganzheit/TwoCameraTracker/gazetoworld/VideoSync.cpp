#include "VideoSync.h"




VideoSync::VideoSync() : Thread() {

    m_bVideoFile = false;
    m_nFPS = 0;

}


bool VideoSync::init(const std::vector<VideoInfo> &info, DualFrameReceiver *_r) {

    if(info.size() != 2) {
        return false;
    }

    frameReceiver = _r;

    /*
     * look for "/dev/video" which is where linux camera devices reside.
     * If not found this is a video file
     */
    m_bVideoFile = strstr(info[0].devname.c_str(), "/dev/video") == NULL;

    // store fps
    m_nFPS = info[0].fps;

    if(m_bVideoFile) { // video file

        // open the eye video file
        capEye.open(info[0].devname);
        if(!capEye.isOpened()) {
            return false;
        }

        // open the scene video file
        capScene.open(info[1].devname);
        if(!capScene.isOpened()) {
            return false;
        }

    }
    else { // camera device

        int w = info[0].w;
        int h = info[0].h;
        Format format = info[0].format;
        int fps = info[0].fps;
        std::string devname = info[0].devname;

        if(!simpleCapEye.open(devname,   // device path
                              w,		 // witdth
                              h,		 // height
                              fps,		 // framerate
                              format)) { // format

            return false;

        }

        w = info[1].w;
        h = info[1].h;
        format = info[1].format;
        fps = info[1].fps;
        devname = info[1].devname;

        if(!simpleCapScene.open(devname,   // device path
                                w,		 // witdth
                                h,		 // height
                                fps,		 // framerate
                                format)) { // format

            return false;

        }

    }

    return true;

}


void VideoSync::run() {

    while(isRunning()) {

        if(m_bVideoFile) { // video file

            /*
             * For video files OpenCV is used instead of gstreamer.
             * TO be accurate, OpenCV uses gstreamer, but it knows how
             * to do that for various video formats.
             */

            cv::Mat imgEye, imgScene;

            // get raw data, OpenCV gives BGR images
            capEye   >> imgEye;
            capScene >> imgScene;

            // check that valid data was received
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
                                              // cv::Mat owns the data

            // create a header for the data. Does not copy data.
            CameraFrame frameScene(w,
                                   h,
                                   bpp,         // bytes per pixel
                                   imgScene.data,
                                   w*h*bpp,
                                   FORMAT_BGR,
                                   false,       // do not copy data
                                   false);      // do not become parent, i.e. do not destroy data in destructor
                                                // cv::Mat owns the data

            frameReceiver->framesReceived(&frameEye, &frameScene);


            double dSleepMs = 1000.0 / m_nFPS;
            sleepMs(dSleepMs);

        }
        else { // camera

            CameraFrame *frameEye   = simpleCapEye.grabFrame(true);
            CameraFrame *frameScene = simpleCapScene.grabFrame(true);

            if(frameEye == NULL || frameScene == NULL) {

                delete frameEye;
                delete frameScene;

                continue;
            }

            frameReceiver->framesReceived(frameEye, frameScene);

            delete frameEye;
            delete frameScene;

        }

        size_t states[2];
		frameReceiver->getWorkerBufferStates(states);

		/* Return the maximum allowed worker buffer queue size */
		int maxBuffSz = frameReceiver->getMaxBufferSize();

        int max = std::max(states[0], states[1]);

        if(max >= maxBuffSz) {
            sleepMs(1000);
        }

    }

}

