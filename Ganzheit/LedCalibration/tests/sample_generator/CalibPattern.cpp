#include "CalibPattern.h"
#include <math.h>
#include <GL/gl.h>


static const double pi		= 3.1415926535897932384626433832795;
static const double radius	= 0.4;
static const int nof_points	= 100;	// including the centre!


CalibPattern::CalibPattern() : GLDrawable() {

	computeVertices();

	/*
	 * Create the pattern. Copy-pasted from LEDCalibPattern, however,
	 * changed to OpenGL coord. system.
	 */
	oglMarkers.resize(16);

	// 0..4 and 8..12
	for(int i = 0; i < 5; ++i) {
		oglMarkers[i]	= Coord2f(-2.0,  2.0 - i);
		oglMarkers[8+i]	= Coord2f( 2.0, -2.0 + i);
	}

	// 5..7 and 13..15
	for(int i = 0; i < 3; ++i) {
		oglMarkers[5+i]	 = Coord2f(-1.0 + i, -2.0);
		oglMarkers[13+i] = Coord2f( 1.0 - i,  2.0);
	}

}


void CalibPattern::computeVertices(void) {

	circleVertices.resize(2 * nof_points);

	// the first fan point is at the centre of the circle
	circleVertices[0] = 0.0;
	circleVertices[1] = 0.0;

	double angRad = 0.0;
	// -1.0 normally, but the centre is included, therefore -2.0
	double angDiff = 2.0*pi / (nof_points - 2.0);
	
	// starting from 1, because 0 is the centre, assigned above
	for(int i = 1; i < nof_points; ++i) {

		circleVertices[2*i]		= radius * cos(angRad);
		circleVertices[2*i+1]	= radius * sin(angRad);

		angRad += angDiff;

	}

}


void CalibPattern::drawCircle(const Coord2f &coord) {

	glPushMatrix();

	// go to coord
	glTranslatef(coord.x, coord.y, 0.0);

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(2,						// number of coordiantes per point
					GL_FLOAT,				// data type
					0,						// offset
					&circleVertices[0]);	// pointer to the data

	glDrawArrays(GL_TRIANGLE_FAN,			// mode
				 0,							// index of first
				 nof_points);				// the number of coordinates

	// deactivate vertex arrays after drawing
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();

}


void CalibPattern::draw() {

	glColor4f(0, 0, 0, 1);

	glPushMatrix();

	glMultMatrixd(transMat);

	std::vector<Coord2f>::iterator it	= oglMarkers.begin();
	std::vector<Coord2f>::iterator end	= oglMarkers.end();

	while(it != end) {

		drawCircle(*it);

		++it;

	}

	glPopMatrix();

}


/* Everything is in the OpenGL coordinate system. */
void CalibPattern::computeNormal(double normal[3]) const {

	/*
	 * Transform the centre. transMat * c
	 * where the centre is (0, 0, 0, 1)
	 */
	double oglCentre[3];
	oglCentre[0] = transMat[12];
	oglCentre[1] = transMat[13];
	oglCentre[2] = transMat[14];


	/*
	 * Transform the normal, which is
	 * (0, 0, 1, 1)
	 */
	normal[0] = transMat[8] + transMat[12];
	normal[1] = transMat[9] + transMat[13];
	normal[2] = transMat[10] + transMat[14];


	normal[0] = normal[0] - oglCentre[0];
	normal[1] = normal[1] - oglCentre[1];
	normal[2] = normal[2] - oglCentre[2];

}


