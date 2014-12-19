#ifndef GLDRAWABLE_H
#define GLDRAWABLE_H


#include <GL/gl.h>
#include "matrix.h"


class GLDrawable {

	public:

		GLDrawable() {

			setIdentity(transMat);

		}

		/* descendants must implement this */
		virtual void draw() = 0;

		/* Move */
		void moveX(double incX) {

			double tr[16];
			setIdentity(tr);
			tr[12] = incX;
			multMatSafe(transMat, tr, transMat);

		}

		/* Move */
		void moveY(double incY) {

			double tr[16];
			setIdentity(tr);
			tr[13] = incY;
			multMatSafe(transMat, tr, transMat);

		}

		/* Move */
		void moveZ(double incZ) {

			double tr[16];
			setIdentity(tr);
			tr[14] = incZ;
			multMatSafe(transMat, tr, transMat);

		}


		void moveXYZ(double incX, double incY, double incZ) {

			double tr[16];
			setIdentity(tr);
			tr[12] = incX;
			tr[13] = incY;
			tr[14] = incZ;
			multMatSafe(transMat, tr, transMat);

		}


		/* Rotate */
		void rotateX(double degX) {

			double tr[16];
			makeXRotMat(tr, degX);

			multMatSafe(transMat, tr, transMat);

		}

		void rotateY(double degY) {

			double tr[16];
			makeYRotMat(tr, degY);

			multMatSafe(transMat, tr, transMat);

		}

		void rotateZ(double degZ) {

			double tr[16];
			makeZRotMat(tr, degZ);

			multMatSafe(transMat, tr, transMat);

		}


		void multFromLeft(double glMat[16]) {

			multMatSafe(glMat, transMat, transMat);

		}

		void multFromRight(double glMat[16]) {

			multMatSafe(transMat, glMat, transMat);

		}


		/* Reset transformation */
		void reset() {

			setIdentity(transMat);

		}


		const double *getTransMat() {return transMat;}


	private:


	protected:

		double transMat[16];


};


#endif

