#include "CameraWidget.h"


CameraWidget::CameraWidget() : GLDrawable() {


}


void CameraWidget::draw() {

	glPushMatrix();

		glMultMatrixd(transMat);

		static const double sz = 0.3;
		static const double sz2 = sz / 2;

		glColor4f(1, 0, 0, 1);

		glBegin(GL_QUADS);

			glVertex2d(-sz2,  sz2);
			glVertex2d(-sz2, -sz2);
			glVertex2d( sz2, -sz2);
			glVertex2d( sz2,  sz2);

		glEnd();

		glBegin(GL_TRIANGLES);

			// left side
			glVertex3d(-sz2,  sz2, 0.0);
			glVertex3d( 0.0,  0.0, sz);
			glVertex3d(-sz2, -sz2, 0.0);

			// bottom side
			glVertex3d(-sz2, -sz2, 0.0);
			glVertex3d( 0.0,  0.0, sz);
			glVertex3d( sz2, -sz2, 0.0);

			// right side
			glVertex3d( sz2, -sz2, 0.0);
			glVertex3d( 0.0,  0.0, sz);
			glVertex3d( sz2,  sz2, 0.0);

			// top side
			glVertex3d(-sz2,  sz2, 0.0);
			glVertex3d( 0.0,  0.0, sz);
			glVertex3d( sz2,  sz2, 0.0);

		glEnd();


	glPopMatrix();

}

