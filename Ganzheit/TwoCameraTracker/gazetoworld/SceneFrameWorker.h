#ifndef SCENE_FRAME_WORKER_H
#define SCENE_FRAME_WORKER_H


#include "JPEGWorker.h"


class SceneFrameWorker : public JPEGWorker {

private:

    CameraFrame *process(CameraFrame *img_compr) {

        CameraFrame *frame;

        if(img_compr->format == FORMAT_MJPG) {

            frame = JPEGWorker::process(img_compr);

        }
        else {

            frame = img_compr;

        }

        return frame;

    }

};


#endif

