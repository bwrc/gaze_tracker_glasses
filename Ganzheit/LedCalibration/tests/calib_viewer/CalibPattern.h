#ifndef CALIBPATTERN_H
#define CALIBPATTERN_H

#include <vector>
#include "GLDrawable.h"


class Coord2f {

	public:

		Coord2f() {x = y = 0.0f;}
		Coord2f(float _x, float _y) {x = _x; y = _y;}

		float x, y;

};


class CalibPattern : public GLDrawable {

	public:

		CalibPattern();

		/* Inherited from GLDrawable */
		void draw();


		void drawCircle(const Coord2f &coord);

		const std::vector<Coord2f> &getOGLMarkers() const {return oglMarkers;}

		void computeNormal(double normal[3]) const;

	private:

		void computeVertices();

		std::vector<float> circleVertices;

		/* These are in the OpenGL coordinate system */
		std::vector<Coord2f> oglMarkers;

};


#endif

