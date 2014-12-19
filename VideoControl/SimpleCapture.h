#ifndef SIMPLECAPTURE_H
#define SIMPLECAPTURE_H


#include "CaptureDevice.h"
#include "CameraFrame.h"
#include <pthread.h>
#include <list>



/*
 * This class enables for capturing frames in a similar manner as OpenCV, i.e.
 * using the grabFrame() function.
 */
class SimpleCapture : FrameReceiver {

public:

    SimpleCapture();

    ~SimpleCapture();


    /*
     * Grab the available frame, block on request.
     * The caller must deallocate the frame by calling delete.
     */
    CameraFrame *grabFrame(bool block);

    //    bool grabFrame(CameraFrame &f, bool block);

    /***************************************************************
     * Open a video device. The device is automatically started
     * to stream data from the file or the camera. In case of a
     * video file stream, only the path is taken into account
     **************************************************************/
    bool open(const std::string &devpath,
              const int width,
              const int height,
              const int framerate,
              const Format format);


    /* Inherited from the FrameReceiver. Called when a frame is ready */
    void frameReceived(const CameraFrame *frame, int id);


    /***************************************************************
     * The frame reader and the frame writer are synchronized with
     * a condition variable
     **************************************************************/
    pthread_cond_t cond_frame;
    pthread_mutex_t mutex_frame;

    CaptureDevice *capdev;

    std::list<CameraFrame *> frameBuffer;

    bool bOpened;

};


#endif

