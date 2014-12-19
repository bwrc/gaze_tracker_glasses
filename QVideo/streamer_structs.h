#ifndef STREAMER_STRUCTS_H
#define STREAMER_STRUCTS_H

//#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>


typedef struct STREAM_INIT_DATA {
	int video_len;
	int buffer_len;
	int w;
	int h;
} STREAM_INIT_DATA;



typedef struct STREAM_FRAME_CHANGED_DATA {
	int framePos;
	int buffState;
//	QWaitCondition *cond;
//	QMutex *mutex;

	pthread_mutex_t *mutex;
	pthread_cond_t *cond;

	cv::Mat *img_rgb;
	bool b_skip_frame;
} STREAM_FRAME_CHANGED_DATA;



#endif
