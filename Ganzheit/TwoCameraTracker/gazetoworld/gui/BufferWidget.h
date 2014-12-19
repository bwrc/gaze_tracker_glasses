#ifndef BUFFERWIDGET_H
#define BUFFERWIDGET_H


#include "GLWidget.h"


namespace gui {


class BufferData {

	public:

		BufferData(int _max_video_frames = 0, int _max_worker_frames = 0) {

			max_video_frames	= max_video_frames;
			max_worker_frames	= max_worker_frames;
			max_save_frames		= max_worker_frames;

			n_video_frames		=
			n_save_frames		=
			n_worker_frames1	=
			n_worker_frames2	= 0;

		}

		int max_video_frames;
		int max_worker_frames;
		int max_save_frames;

		int n_video_frames;
		int n_save_frames;
		int n_worker_frames1;
		int n_worker_frames2;

};



class BufferWidget : public GLWidget {

	public:

		BufferWidget(const View &_view);

		void draw(const BufferData &data);

	private:

};


}


#endif

