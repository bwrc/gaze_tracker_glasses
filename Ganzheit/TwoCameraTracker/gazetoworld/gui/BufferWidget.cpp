#include "BufferWidget.h"
#include <algorithm>


namespace gui {


static const int BORDER_W = 9;


BufferWidget::BufferWidget(const View &_view) : GLWidget(_view) {

}


void BufferWidget::draw(const BufferData &data) {

	GLWidget::draw();

	glColor4f(1, 1, 1, 1);

	glBegin(GL_QUADS);

		const double x		= BORDER_W;
		double y			= BORDER_W;
		const double max_w	= view.w - 2*BORDER_W;
		const double max_h	= (view.h - 5*BORDER_W) / 4.0;

		/********************************************************************
		 * Draw the video buffer state
		 ********************************************************************/
		double numerator	= (double)data.n_video_frames;
		double denominator	= std::max((double)data.max_video_frames, 1.0);
		double ratio		= numerator / denominator;
		double wbar			= ratio * (view.w - 2*BORDER_W);
		double hbar			= (view.h - 5*BORDER_W) / 4.0;

		// paint the background
		glColor4f(1, 1, 1, 1);
		glVertex2i(x,		  y + max_h);
		glVertex2i(x,		  y);
		glVertex2i(x + max_w, y);
		glVertex2i(x + max_w, y + max_h);

		// paint the bar
		glColor4f((float)ratio, .0, .3, 1.0);
		glVertex2i(x,		 y + hbar);
		glVertex2i(x,		 y);
		glVertex2i(x + wbar, y);
		glVertex2i(x + wbar, y + hbar);



		/********************************************************************
		 * Draw the save buffer state
		 ********************************************************************/
		numerator	= (double)data.n_save_frames;
		denominator	= std::max((double)data.max_save_frames, 1.0);
		ratio		= numerator / denominator;
		y			+= hbar + BORDER_W;
		wbar		= ratio * (view.w - 2*BORDER_W);

		// paint the background
		glColor4f(1, 1, 1, 1);
		glVertex2i(x,		  y + max_h);
		glVertex2i(x,		  y);
		glVertex2i(x + max_w, y);
		glVertex2i(x + max_w, y + max_h);

		// paint the bar
		glColor4f((float)ratio, .0, .3, 1.0);
		glVertex2i(x,		 y + hbar);
		glVertex2i(x,		 y);
		glVertex2i(x + wbar, y);
		glVertex2i(x + wbar, y + hbar);



		/********************************************************************
		 * Draw the state of worker 1
		 ********************************************************************/
		numerator	= (double)data.n_worker_frames1;
		denominator	= std::max((double)data.max_worker_frames, 1.0);
		ratio		= numerator / denominator;
		y			+= hbar + BORDER_W;
		wbar		= ratio * (view.w - 2*BORDER_W);

		// paint the background
		glColor4f(1, 1, 1, 1);
		glVertex2i(x,		  y + max_h);
		glVertex2i(x,		  y);
		glVertex2i(x + max_w, y);
		glVertex2i(x + max_w, y + max_h);

		// paint the bar
		glColor4f((float)ratio, .0, .3, 1.0);
		glVertex2i(x,		 y + hbar);
		glVertex2i(x,		 y);
		glVertex2i(x + wbar, y);
		glVertex2i(x + wbar, y + hbar);


		/********************************************************************
		 * Draw the state of worker 2
		 ********************************************************************/
		numerator	= (double)data.n_worker_frames2;
		denominator	= std::max((double)data.max_worker_frames, 1.0);
		ratio		= numerator / denominator;
		y			+= hbar + BORDER_W;
		wbar		= ratio * (view.w - 2*BORDER_W);

		// paint the background
		glColor4f(1, 1, 1, 1);
		glVertex2i(x,		  y + max_h);
		glVertex2i(x,		  y);
		glVertex2i(x + max_w, y);
		glVertex2i(x + max_w, y + max_h);

		// paint the bar
		glColor4f((float)ratio, .0, .3, 1.0);
		glVertex2i(x,		 y + hbar);
		glVertex2i(x,		 y);
		glVertex2i(x + wbar, y);
		glVertex2i(x + wbar, y + hbar);

		glColor4f(1, 1, 1, 1);

	glEnd();

}


}

