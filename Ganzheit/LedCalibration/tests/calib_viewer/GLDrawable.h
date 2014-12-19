#ifndef GLDRAWABLE_H
#define GLDRAWABLE_H


#include <GL/gl.h>
#include "matrix.h"
#include <string.h>


class View {

	public:

		View(int _x = 0, int _y = 0,
			 int _w = 0, int _h = 0,
			 bool _b_ortho = false,
			 double _FOV_y = 0.0,
			 double _znear = 1.0,
			 double _zfar = 50.0,
			 double *_intr = NULL) {

			x = _x;
			y = _y;
			w = _w;
			h = _h;
			b_ortho = _b_ortho;
			znear = _znear;
			zfar = _zfar;

			if(_intr != NULL) {
				memcpy(intr, _intr, 9*sizeof(double));
			}

		}

		int x;
		int y;
		int w;
		int h;
		bool b_ortho;
		double znear;
		double zfar;

		/* Column-major order */
		double intr[9];

	static void selectView(const View &view) {

		int x = view.x;
		int y = view.y;
		int w = view.w;
		int h = view.h;

		glViewport(x, y, w, h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		if(view.b_ortho) {
			glOrtho(x,			// left
					w + x-1,	// right
					y,			// bottom
					h + y-1,	// top
					-1,			// near
					1);			// far
		}
		else {

			/***************************************************************
			 * http://strawlab.org/2011/11/05/augmented-reality-with-OpenGL/
			 ***************************************************************/

			double mat[16];

			double znear = view.znear;
			double zfar = view.zfar;
			double x0 = 0;
			double y0 = 0;
			const double *intr = view.intr;

			// Row 1
			mat[0] = 2.0*intr[0] / view.w;
			mat[4] = -2.0*intr[3] / view.h;
			mat[8] = (view.w - 2*intr[6] + 2*x0) / view.w;
			mat[12] = 0;

			// Row 2
			mat[1] = 0;
			mat[5] = 2*intr[4]/(view.h);
			mat[9] = (-(view.h) + 2*intr[7] + 2*y0) / (view.h);
			mat[13] = 0;

			// Row 3
			mat[2] = 0;
			mat[6] = 0;
			mat[10] = (-zfar-znear) / (zfar - znear);
			mat[14] = -2*zfar*znear / (zfar-znear);

			// Row 4
			mat[3] = 0;
			mat[7] = 0;
			mat[11] = -1;
			mat[15] = 0;

			glLoadMatrixd(mat);

		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

	}

};




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


		const double *getTransMat() const {return transMat;}


	private:


	protected:

		double transMat[16];


};


#endif

