#include "LED.h"



LED::LED() : GLDrawable() {

}


void LED::draw() {

	glPushMatrix();

		glMultMatrixd(transMat);

		glColor4f(0, 0, 1, 1);

		glBegin(GL_QUADS);

			static const double sz = 0.3;
			static const double sz2 = sz / 2.0;

			// front face
			glVertex3d(-sz2,  sz2, sz2);
			glVertex3d(-sz2, -sz2, sz2);
			glVertex3d( sz2, -sz2, sz2);
			glVertex3d( sz2,  sz2, sz2);

			// left face
			glVertex3d(-sz2,  sz2, -sz2);
			glVertex3d(-sz2, -sz2, -sz2);
			glVertex3d(-sz2, -sz2, sz2);
			glVertex3d(-sz2,  sz2, sz2);

			// back face
			glVertex3d( sz2,  sz2, -sz2);
			glVertex3d( sz2, -sz2, -sz2);
			glVertex3d(-sz2, -sz2, -sz2);
			glVertex3d(-sz2,  sz2, -sz2);

			// right face
			glVertex3d(sz2,  sz2,  sz2);
			glVertex3d(sz2, -sz2,  sz2);
			glVertex3d(sz2, -sz2, -sz2);
			glVertex3d(sz2,  sz2, -sz2);

			// top face
			glVertex3d(-sz2, sz2, -sz2);
			glVertex3d(-sz2, sz2,  sz2);
			glVertex3d( sz2, sz2,  sz2);
			glVertex3d( sz2, sz2, -sz2);

			// bottom face
			glVertex3d(-sz2, -sz2,  sz2);
			glVertex3d(-sz2, -sz2, -sz2);
			glVertex3d( sz2, -sz2, -sz2);
			glVertex3d( sz2, -sz2,  sz2);

		glEnd();

	glPopMatrix();

}

