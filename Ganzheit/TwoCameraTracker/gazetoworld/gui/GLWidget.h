#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>



namespace gui {


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
		double intr[9];
    };


    class GLWidget {

	public:

		GLWidget(const View &_view);

		virtual ~GLWidget();

		virtual void draw();


	private:
		static void select_view(const View &view) {

			const int x = view.x;
			const int y = view.y;
			const int w = view.w;
			const int h = view.h;

			glViewport(x, y, w, h);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			if(view.b_ortho) {
				glOrtho(0,		// left
						w-1,	// right
						0,		// bottom
						h-1,	// top
						-1,		// near
						1);		// far
			}
			else {

				double mat[16];

				double znear = 1.0;
				double zfar = 50.0;
				double x0 = 0;
				double y0 = 0;
				const double *intr = view.intr;

				// Row 1
				mat[0] = 2.0*intr[0] / view.w;
				mat[4] = -2.0*intr[1] / view.h;
				mat[8] = (view.w - 2*intr[2] + 2*x0) / view.w;
				mat[12] = 0;

				// Row 2
				mat[1] = 0;
				mat[5] = 2*intr[4]/(view.h);
				mat[9] = (-(view.h) + 2*intr[5] + 2*y0) / (view.h);
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


	protected:
		View view;

    };


}


#endif

