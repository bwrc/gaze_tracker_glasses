#include "Pupil.h"



Pupil::Pupil(int _radius, int _x, int _y) {
	radius = _radius;

	pos_x = _x;
	pos_y = _y;
}


void Pupil::moveTo(int x, int y) {
	pos_x = x;
	pos_y = y;
}


void Pupil::moveX(int inc_x) {
	pos_x += inc_x;
}


void Pupil::moveY(int inc_y) {
	pos_y += inc_y;
}


void Pupil::getPos(int &x, int &y) const {
	x = pos_x;
	y = pos_y;
}

