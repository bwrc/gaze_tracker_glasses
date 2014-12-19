#include "PanelIdle.h"
#include <stdio.h>


namespace gui {



PanelIdle::PanelIdle(const View &_view) : GLWidget(_view) {

}


PanelIdle::~PanelIdle() {

}


void PanelIdle::draw() {

	GLWidget::draw();


	float w = view.w / 2.0f;
	float h = view.h / 2.0f;
	float w2 = w / 2.0f;
	float h2 = h / 2.0f;

	glLoadIdentity();

	glTranslatef(w, h, 0);

	static float ang = 0.0f;
	glRotatef(ang, 0, 0, 1);
	ang += 1.0f;

	glColor4f(.2f, .5f, .2f, 1.0f);
	glBegin(GL_QUADS);

		glVertex2f(-w2, h2);
		glVertex2f(-w2, -h2);
		glVertex2f(w2, -h2);
		glVertex2f(w2, h2);

	glEnd();

}


}

