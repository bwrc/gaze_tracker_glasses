#ifndef GLVIDEOCANVAS_H
#define GLVIDEOCANVAS_H


#include <string.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "CameraFrame.h"
#include "GLWidget.h"




namespace gui {


class GLVideoCanvas : public GLWidget {

	public:

		GLVideoCanvas(const View &_view);
		~GLVideoCanvas();

		void draw(const CameraFrame *img);


	private:
		GLuint texture;


};


}


#endif

