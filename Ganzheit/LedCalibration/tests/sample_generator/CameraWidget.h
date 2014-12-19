#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H


#include "GLDrawable.h"


class CameraWidget : public GLDrawable {

	public:

		CameraWidget();

		/* Inherited from GLDrawable */
		void draw();

};


#endif

