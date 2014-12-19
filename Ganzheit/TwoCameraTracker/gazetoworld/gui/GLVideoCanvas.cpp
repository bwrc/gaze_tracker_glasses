#include "GLVideoCanvas.h"



namespace gui {



    GLVideoCanvas::GLVideoCanvas(const View &_view) : GLWidget(_view) {

        /******************************************************
         * create the texture for the instructions
         ******************************************************/
        glEnable(GL_TEXTURE_2D);

        glGenTextures(1, &texture);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    }


    GLVideoCanvas::~GLVideoCanvas() {
        glDeleteTextures(1, &texture);
    }


    void GLVideoCanvas::draw(const CameraFrame *img) {

        GLWidget::draw();

        /***************************************************************
         *
         ***************************************************************/
        glEnable(GL_TEXTURE_2D);

        // draw the results to the lower view
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, 3, img->w, img->h, 0, GL_BGR, GL_UNSIGNED_BYTE, img->data);

        glBegin(GL_QUADS);
		glTexCoord2d(0.0, 0.0); glVertex2d(0,			view.h-1);	// upper left
		glTexCoord2d(0.0, 1.0); glVertex2d(0,			0);			// lower left
		glTexCoord2d(1.0, 1.0); glVertex2d(view.w-1,	0);			// lower right
		glTexCoord2d(1.0, 0.0); glVertex2d(view.w-1,	view.h-1);	// upper right
        glEnd();


        glDisable(GL_TEXTURE_2D);

    }


}

