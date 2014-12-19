#include <sys/time.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>
#include "CalibPattern.h"
#include "LED.h"
#include "CameraWidget.h"
#include "Camera.h"
#include "LEDCalibrator.h"
#include "LEDCalibPattern.h"
#include "CalibDataWriter.h"


static const int WIDTH					= 640;
static const int HEIGHT					= 960;
static const double SPEED				= 0.2;
static const double SPEED_ROT			= 1.2;
static const double DEFAULT_PATTERN_Z	= -25;
static const int DBL_PRINT_ACCURACY     = 8;


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




/**********************************************************
 * Globals
 **********************************************************/
CalibPattern pattern;
CameraWidget cameraWidget;
LED ledWidget;
Camera cam;

// the sample container
calib::LEDCalibContainer samples;

// the current sample
calib::LEDCalibSample curSample;

// The upper and lower view
View viewScene;		// the 3D scene
View viewCamera;	// what the camera sees


/**********************************************************
 * Prototypes
 **********************************************************/
void cvVecToGlVec(const double cvVec[3], double glVec[3]);
void draw();
bool computeGlintPos(double vReflect[3]);
void initGL();
bool init_all(SDL_Surface **screen);
bool initSDL(SDL_Surface **screen);
int handle_events();
void main_loop();
void calibrate();
void addSample();
void handleDebouncableAction(const unsigned char *keystate);


void quit() {

	/* clean up the window */
    SDL_Quit();

}


bool init_all(SDL_Surface **screen) {

	if(!initSDL(screen)) {
		quit();
		return false;
	}

	initGL();


	/****************************************************************
	 * Set the OpenGL views
	 ****************************************************************/
	{

		double h = HEIGHT/2.0;

		// upper view
		viewScene.x			= 0;
		viewScene.y			= h;
		viewScene.w			= WIDTH;
		viewScene.h			= h;
		viewScene.b_ortho	= false;
		viewScene.znear		= 0.1;
		viewScene.zfar		= 50.0;

		// ideal
		static const double FOV_y	= 90.0;
		static const double cx		= (WIDTH - 1.0) / 2.0;
		static const double cy		= (h - 1.0) / 2.0;
		static const double fy		= (h/2.0) / tan(DEGTORAD(FOV_y / 2.0));
		static const double fx		= fy;

		/*
		 * column-major 
		 *
		 *   fx,    0,     cx,
		 *   0,     fy,    cy,
		 *   0,     0,     1
		 */
		double intrScene[9] = {fx, 0.0, 0.0, 0.0, fy, 0.0, cx, cy, 1.0};
		memcpy(viewScene.intr, intrScene, 9*sizeof(double));


		// bottom view
		viewCamera.x		= 0;
		viewCamera.y		= 0;
		viewCamera.w		= WIDTH;
		viewCamera.h		= HEIGHT / 2;
		viewCamera.b_ortho	= true;
		viewCamera.znear	= 0.1;	// not used
		viewCamera.zfar		= 1;	// not used

	}


	/****************************************************************
	 * Set the camera parameters
	 ****************************************************************/
	{

		double h = HEIGHT/2.0;

		static const double FOV_y	= 60.0;
		static const double cx		= (WIDTH - 1.0) / 2.0;
		static const double cy		= (h - 1.0) / 2.0;
		static const double fy		= (h/2.0) / tan(DEGTORAD(FOV_y / 2.0));
		static const double fx		= fy;


		/*
		 * column-major 
		 *
		 *   fx,    0,     cx,
		 *   0,     fy,    cy,
		 *   0,     0,     1
		 */
		double intrCam[9] = {fx, 0.0, 0.0, 0.0, fy, 0.0, cx, cy, 1.0};
		double distCam[5] = {0, 0, 0, 0, 0};

		cam.setIntrinsicMatrix(intrCam);
		cam.setDistortion(distCam);

	}

	// place the widgets into the scene
	pattern.moveZ(DEFAULT_PATTERN_Z);
	cameraWidget.moveXYZ(-0.2, -0.5, -3);
	ledWidget.moveXYZ(1.5, 0.9, -1.8);


	// initialise the sample container

	samples = calib::LEDCalibContainer();
	samples.circleSpacing = 1.0;

	return true;

}


bool initSDL(SDL_Surface **screen) {

	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	*screen = SDL_SetVideoMode(WIDTH, HEIGHT, 24, SDL_OPENGL);

	if(!(*screen)) {
		printf("screen is NULL\n");
		return false;
	}

	return true;

}


void initGL() {

	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Test To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations, decreases perfromance though

	glEnable(GL_LIGHTING);

}


int main(int argc, char *argv[]) {

	SDL_Surface *screen = NULL;

	if(!init_all(&screen)) {
		SDL_FreeSurface(screen);
		quit();

		return -1;
	}

	// run the main loop
	main_loop();


	SDL_FreeSurface(screen);

	quit();

	return 0;

}


void main_loop() {

	int running = 1;

	while(running) {

		// clear The Screen And The Depth Buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// handle events
		running = handle_events();

		// draw scene
		draw();

		// display contents
		SDL_GL_SwapBuffers();

	}

}


void draw() {

    curSample.clear();

	/*****************************************************************
	 * Draw the horisontal middle line
	 *****************************************************************/
	glViewport(0, 0, WIDTH, HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,			// left
			WIDTH-1,	// right
			0,			// bottom
			HEIGHT-1,	// top
			-1,			// near
			1);			// far
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor4f(0, 0, 0, 1);
	glBegin(GL_LINES);
		glVertex2f(0, HEIGHT/2);
		glVertex2f(WIDTH, HEIGHT/2);
	glEnd();


	// select the scene view
	View::selectView(viewScene);

	/*****************************************************************
	 * Draw the widgets and the pattern points
	 *****************************************************************/
	pattern.draw();
	ledWidget.draw();
	cameraWidget.draw();


	/*****************************************************************
	 * Camera position in the world
	 *****************************************************************/
	const double *trCam = cameraWidget.getTransMat();
	const double vCam[3] = {trCam[12], trCam[13], trCam[14]};


	/*****************************************************************
	 * LED position in the world
	 *****************************************************************/
	const double *trLED = ledWidget.getTransMat();
	const double vLED[3] = {trLED[12], trLED[13], trLED[14]};


	double invTrCam[16];
	bool b_resInvert = invert(trCam, invTrCam);

	if(!b_resInvert) {

		return;

	}


	// compute the position of the reflection, in OpenGL coord. system
	double vReflection[3];
	bool success = computeGlintPos(vReflection);

	if(success) {

		/*****************************************************************
		 * Draw the lines
		 *****************************************************************/
		glColor4f(0, 0, 0, 1);
		glLineWidth(2.0);
		glBegin(GL_LINES);

			// cam to reflection
			glVertex3dv(vCam);
			glVertex3dv(vReflection);

			// reflection to LED
			glVertex3dv(vReflection);
			glVertex3dv(vLED);

		glEnd();

	}

	// select the camera's view
	View::selectView(viewCamera);
	glPointSize(10);

	if(success) {

		/*****************************************************************
		 * Convert the point of relflection and the pattern points into 2D
		 * coordinates in the camera's coordinate system
		 *****************************************************************/

		double vCamReflection[3] = {invTrCam[0] * vReflection[0] +
									invTrCam[4] * vReflection[1] +
									invTrCam[8] * vReflection[2] +
									invTrCam[12],

									invTrCam[1] * vReflection[0] +
									invTrCam[5] * vReflection[1] +
									invTrCam[9] * vReflection[2] +
									invTrCam[13],

									invTrCam[2] * vReflection[0] +
									invTrCam[6] * vReflection[1] +
									invTrCam[10] * vReflection[2] +
									invTrCam[14]};

		// convert to OpenCV coordinate system
		double ocvX =  vCamReflection[0];
		double ocvY = -vCamReflection[1];
		double ocvZ = -vCamReflection[2];

		cv::Point3d p3D(ocvX, ocvY, ocvZ);
		double ocvU, ocvV;
		cam.worldToPix(p3D, &ocvU, &ocvV);


		glColor4f(1, 0, 0, 1);
		glBegin(GL_POINTS);
			glVertex2f(ocvU, viewCamera.h - ocvV - 1);
		glEnd();

		curSample.glint = cv::Point2d(ocvU, ocvV);

	}


	// the trans. mat of the pattern
	const double *oglTrPattern = pattern.getTransMat();

	/*
	 * Converting a transformation matrix from OpenCV to OpenGL
	 * or vice versa is a matter of multiplying either matrix
	 * from the left hand side by a rotation matrix defined as:
	 *
	 *          | 1       0       0       0 |
	 * Rx(a)  = | 0     cos(a) -sin(a)    0 |
	 *          | 0     sin(a)  cos(a)    0 |
	 *          | 0       0       0       1 |
	 *
	 *              =>
	 *
	 *          | 1       0       0       0 |
	 * Rx(pi) = | 0      -1       0       0 |
	 *          | 0       0      -1       0 |
	 *          | 0       0       0       1 |
	 *
	 * i.e. a rotation by pi (180) around the x axis.
	 */

    /*
     * Uncomment to get the "real" transformation matrix 
     *
     * // express the pattern's trans. matrix in the camera's coord. system
     * double tmp[16];
     * multMatSafe(invTrCam, oglTrPattern, tmp);
     *
     * cv::Mat tm(4, 4, CV_32FC1);
     *
     * // now do the rotation by pi (180) around the x-axis
     * tm.at<float>(0, 0) =  tmp[0];
     * tm.at<float>(1, 0) = -tmp[1];
     * tm.at<float>(2, 0) = -tmp[2];
     * tm.at<float>(3, 0) =  tmp[3];
     * tm.at<float>(0, 1) =  tmp[4];
     * tm.at<float>(1, 1) = -tmp[5];
     * tm.at<float>(2, 1) = -tmp[6];
     * tm.at<float>(3, 1) =  tmp[7];
     * tm.at<float>(0, 2) =  tmp[8];
     * tm.at<float>(1, 2) = -tmp[9];
     * tm.at<float>(2, 2) = -tmp[10];
     * tm.at<float>(3, 2) =  tmp[11];
     * tm.at<float>(0, 3) =  tmp[12];
     * tm.at<float>(1, 3) = -tmp[13];
     * tm.at<float>(2, 3) = -tmp[14];
     * tm.at<float>(3, 3) =  tmp[15];
     */

    /*
     * Uncomment to get the "real" normal
     *
     * the pattern's normal in the world coord. system
     * double oglNormal[4];
     * pattern.computeNormal(oglNormal);
     * oglNormal[3] = 1;
     *
     * // the pattern's normal in the camera coord. system
     * double oglNormalCam[4];
     * multMatVec(invTrCam, oglNormal, oglNormalCam);
     *
     * // the world origin in the camera's coord. system
     * double worldOriginInCam[3] = {invTrCam[12], invTrCam[13], invTrCam[14]};
     *
     * oglNormalCam[0] = oglNormalCam[0] - worldOriginInCam[0];
     * oglNormalCam[1] = oglNormalCam[1] - worldOriginInCam[1];
     * oglNormalCam[2] = oglNormalCam[2] - worldOriginInCam[2];
     *
     * // convert to OpenCV coord. system and put to the sample
     * curSample.normal.x =  oglNormalCam[0];
     * curSample.normal.y = -oglNormalCam[1];
     * curSample.normal.z = -oglNormalCam[2];
     */

	glColor4f(0, 0, 0, 1);
	glBegin(GL_POINTS);

	/* NOTE: These points are in the OpenGL coord. system. */
	const std::vector<Coord2f> oglMarkers = pattern.getOGLMarkers();

    curSample.image_points.resize(16);

	for(size_t i = 0; i < oglMarkers.size(); ++i) {

		// get the current point, in the pattern's coord. system
		const Coord2f &oglCurPoint = oglMarkers[i];

		// transform into the world coord. system
		double pw[3] = {
			oglTrPattern[0] * oglCurPoint.x + oglTrPattern[4] * oglCurPoint.y + oglTrPattern[12],
			oglTrPattern[1] * oglCurPoint.x + oglTrPattern[5] * oglCurPoint.y + oglTrPattern[13],
			oglTrPattern[2] * oglCurPoint.x + oglTrPattern[6] * oglCurPoint.y + oglTrPattern[14]};

		// then to the camera's coord. system
		const double vCamPoint[3] = {invTrCam[0]  * pw[0] +
									 invTrCam[4]  * pw[1] +
									 invTrCam[8]  * pw[2] +
									 invTrCam[12],

									 invTrCam[1]  * pw[0] +
									 invTrCam[5]  * pw[1] +
									 invTrCam[9]  * pw[2] +
									 invTrCam[13],

									 invTrCam[2]  * pw[0] +
									 invTrCam[6]  * pw[1] +
									 invTrCam[10] * pw[2] +
									 invTrCam[14]};

		// convert to OpenCV coordinate system
		double ocvX =  vCamPoint[0];
		double ocvY = -vCamPoint[1];
		double ocvZ = -vCamPoint[2];

		cv::Point3d p3D(ocvX, ocvY, ocvZ);
		double ocvU, ocvV;
		cam.worldToPix(p3D, &ocvU, &ocvV);

		/* The locations of the found circles */
		curSample.image_points[i] = cv::Point2f(ocvU, ocvV);

		glVertex2f(ocvU, viewCamera.h - ocvV - 1); 

	}

	glEnd();


}


/*
 *                                 D
 *                  |-----------------------------|
 *
 *                          k2              k2'
 *                  |-----------------|-----------|
 *     ___________________________________________________________
 *                  |        alpha ( /|\ ) alpha  |
 *                  |               / | \         |
 *                  |              /  |  \        |
 *                  |             /   |   \       |
 *                  |            /    |    \      |
 *                  |           /     |     \     |
 *               k1 |          /      |      \    | k1'
 *                  |         /       |       \   |
 *                  |        /        n        \  |
 *                  |       /                   \ |
 *                  |      /                     \|
 *                  |     /                      LED
 *                  |    /
 *                  |   /
 *                  |  /
 *                  | /
 *                  |/
 *                 Cam
 *
 *
 *         O
 *    World origin
 *
 *
 *       |----------------------------------|
 *       |               Math               |
 *       |----------------------------------|
 *       | tan(alpha) = k1 / k2 = k1' / k2' |
 *       |                                  |
 *       |              =>                  |
 *       |                                  |
 *       | k1 / k2 = k1' / (D - k2)         |
 *       |                                  |
 *       |              =>                  |
 *       |                                  |
 *       | k2 = (k1 * D) / (k1' + k1)       |
 *       |----------------------------------|
 *
 */
bool computeGlintPos(double vReflection[3]) {

	/*****************************************************************
	 * Surface normal
	 *****************************************************************/
	double normal[3];
	pattern.computeNormal(normal);


	/*****************************************************************
	 * Camera position
	 *****************************************************************/
	const double *trCam = cameraWidget.getTransMat();
	double vCam[3] = {trCam[12], trCam[13], trCam[14]};


	/*****************************************************************
	 * LED position
	 *****************************************************************/
	const double *trLED = ledWidget.getTransMat();
	double vLED[3] = {trLED[12], trLED[13], trLED[14]};


	/*****************************************************************
	 * Compute the shortest distance from the camera to the surface
	 *****************************************************************/

	/*
	 * Get the first marker. This is (-2, 2, 0) and is defined
	 * in the OpenGL coordinate system as the upper left marker.
	 */
	const Coord2f P02f = pattern.getOGLMarkers()[0];
	const double *trPattern = pattern.getTransMat();
	double P0[3] = {trPattern[0]*P02f.x + trPattern[4]*P02f.y + trPattern[12],
					trPattern[1]*P02f.x + trPattern[5]*P02f.y + trPattern[13],
					trPattern[2]*P02f.x + trPattern[6]*P02f.y + trPattern[14]};

	// vector from P0 to the cam
	double vP0Cam[3] = {vCam[0] - P0[0],
						vCam[1] - P0[1],
						vCam[2] - P0[2]};

	double k1 = dot(normal, vP0Cam) / dot(normal, normal);


	/*****************************************************************
	 * Compute the shortest distance from the LED to the surface
	 *****************************************************************/

	// vector from P0 to the LED
	double vP0LED[3] = {vLED[0] - P0[0],
						vLED[1] - P0[1],
						vLED[2] - P0[2]};

	double k1_prime = dot(normal, vP0LED) / dot(normal, normal);


	/*****************************************************************
	 * Compute the distance between the intersections of the plane and
	 * the camera and the plane and the LED.
	 *****************************************************************/

	// define a vector to the intersection of the plane and the camera
	double vCamPlane[3] = {vCam[0] + (-k1*normal[0]),
						   vCam[1] + (-k1*normal[1]),
						   vCam[2] + (-k1*normal[2])};

	// define a vector to the intersection of the plane and the LED
	double vLEDPlane[3] = {vLED[0] + (-k1_prime*normal[0]),
						   vLED[1] + (-k1_prime*normal[1]),
						   vLED[2] + (-k1_prime*normal[2])};


	// compute the distance between them
	double D = vecToVecDist(vCamPlane, vLEDPlane);


	/*****************************************************************
	 * Compute the distance from the cam-plane intersection to the
	 * point of reflection
	 *****************************************************************/
	double k2 = (k1 * D) / (k1_prime + k1);


	/*****************************************************************
	 * Compute the point of the reflection
	 *****************************************************************/

	// define vD = vLEDPlane - vCamPlane
	double vD[3] = {vLEDPlane[0] - vCamPlane[0],
					vLEDPlane[1] - vCamPlane[1],
					vLEDPlane[2] - vCamPlane[2]};

	// normalise it
	normalise(vD);

	// compute the position vector of the point of reflection
	vReflection[0] = vCamPlane[0] + k2*vD[0];
	vReflection[1] = vCamPlane[1] + k2*vD[1];
	vReflection[2] = vCamPlane[2] + k2*vD[2];


	/*****************************************************************
	 * See if the point is inside the rectangle
	 *****************************************************************/
	double trInvPattern[16];
	bool b_invSuccess = invert(trPattern, trInvPattern);

	if(b_invSuccess) {

		/*
		 * Compute the position of the reflection in the
		 * pattern's coordinate system. No need to compute
		 * the z-coordinate since the point is on the surface
		 * of the pattern, i.e. z = 0.
		 */
		double p[2] = {
					   // x
					   trInvPattern[0] * vReflection[0] +
					   trInvPattern[4] * vReflection[1] +
					   trInvPattern[8] * vReflection[2] +  
					   trInvPattern[12],

					   // y
					   trInvPattern[1] * vReflection[0] +
					   trInvPattern[5] * vReflection[1] +
					   trInvPattern[9] * vReflection[2] +  
					   trInvPattern[13]
					   };


		// get the markers
		const std::vector<Coord2f> &oglMarkers = pattern.getOGLMarkers();

		// counter clock-wise, convert to OpenGL coordinate system
		Coord2f oglc1 = oglMarkers[0];	// left up
		Coord2f oglc2 = oglMarkers[4];	// left down
		Coord2f oglc3 = oglMarkers[8];	// right down
		Coord2f oglc4 = oglMarkers[12];	// right up

		double minLeft  = oglc1.x < oglc2.x ? oglc1.x : oglc2.x;
		double maxRight = oglc3.x > oglc4.x ? oglc3.x : oglc4.x;
		double maxUp    = oglc1.y > oglc4.y ? oglc1.y : oglc4.y;
		double minDown  = oglc2.y < oglc3.y ? oglc2.y : oglc3.y;

		// if the reflection is inside the rectangle, return success
		if(p[0] >= minLeft && p[0] <= maxRight &&
		   p[1] >= minDown && p[1] <= maxUp) {

			return true;

		}

	}

	return false;

}


int handle_events() {

	SDL_Event evt;

	int running = 1;

	while(SDL_PollEvent(&evt)) {
		switch(evt.type){
			case SDL_KEYDOWN: {
				if(evt.key.keysym.sym == SDLK_ESCAPE) {
					running = 0;
				}

				break;
			}
			case SDL_QUIT: {
				running = 0;
				break;
			}
		}
	}

	const unsigned char *keystate = SDL_GetKeyState(NULL);


	double tr[16];
	setIdentity(tr);

	if(keystate[SDLK_z]) {

		pattern.moveZ(SPEED);

	}
	else if(keystate[SDLK_x]) {

		pattern.moveZ(-SPEED);

	}
	if(keystate[SDLK_UP]) {

		pattern.moveY(SPEED);

	}
	else if(keystate[SDLK_DOWN]) {

		pattern.moveY(-SPEED);

	}
	if(keystate[SDLK_RIGHT]) {

		pattern.moveX(SPEED);

	}
	else if(keystate[SDLK_LEFT]) {

		pattern.moveX(-SPEED);

	}
	if(keystate[SDLK_s]) {

		pattern.rotateX(SPEED_ROT);

	}
	else if(keystate[SDLK_w]) {

		pattern.rotateX(-SPEED_ROT);

	}
	if(keystate[SDLK_a]) {

		pattern.rotateY(-SPEED_ROT);

	}
	else if(keystate[SDLK_d]) {

		pattern.rotateY(SPEED_ROT);

	}
	if(keystate[SDLK_q]) {

		pattern.rotateZ(SPEED_ROT);

	}
	else if(keystate[SDLK_e]) {

		pattern.rotateZ(-SPEED_ROT);

	}

	if(keystate[SDLK_r]) {

		pattern.reset();
		pattern.moveZ(DEFAULT_PATTERN_Z);

	}

	bool b_debouncable = keystate[SDLK_RETURN] || keystate[SDLK_c] || keystate[SDLK_o];


	if(b_debouncable) {
		handleDebouncableAction(keystate);
	}

	return running;

}


void handleDebouncableAction(const unsigned char *keystate) {

	static struct timeval t1 = (struct timeval){0};
	struct timeval t2;
	gettimeofday(&t2, NULL);

	long sec	= t2.tv_sec - t1.tv_sec;
	long usec	= t2.tv_usec - t1.tv_usec;

	double millis = sec * 1000.0 + usec / 1000.0;

	if(millis > 1000) {

		if(keystate[SDLK_RETURN]) {

			addSample();

		}
		else if(keystate[SDLK_c]) {

			calibrate();

		}
        else if(keystate[SDLK_o]) {

            CalibDataWriter writer;
            writer.create("test.xml");

            calib::CameraCalibContainer cameraData;

            /* Object points */
            calib::CameraCalibrator::getObjectPoints(cameraData.object_points);

            /* The obtained intrinsic matrix. Column-major order. */
            cv::Mat camIntr = cam.getIntrisicMatrix();

            cameraData.intr[0] = camIntr.at<double>(0, 0);
            cameraData.intr[1] = camIntr.at<double>(1, 0);
            cameraData.intr[2] = camIntr.at<double>(2, 0);
            cameraData.intr[3] = camIntr.at<double>(0, 1);
            cameraData.intr[4] = camIntr.at<double>(1, 1);
            cameraData.intr[5] = camIntr.at<double>(2, 1);
            cameraData.intr[6] = camIntr.at<double>(0, 2);
            cameraData.intr[7] = camIntr.at<double>(1, 2);
            cameraData.intr[8] = camIntr.at<double>(2, 2);


            /* The obtained distortion coefficient vector */
            cameraData.dist[0] = cameraData.dist[1] = cameraData.dist[2] =
            cameraData.dist[3] = cameraData.dist[4] = 0.0;

            /* The obtained reprojection error */
            cameraData.reproj_err = 0.0;

            /* The size of the image */
            cameraData.imgSize = cv::Size(WIDTH, HEIGHT);

            std::vector<calib::LEDCalibContainer> LEDData(1);
            LEDData[0] = samples;


            writer.writeCameraData(cameraData);
            writer.writeLEDData(LEDData);

            printf("XML file written\n");

        }

		t1 = t2;

	}

}


void addSample() {

    curSample.img_path = std::string("dummy_path.jpg");
	samples.addSample(curSample);

	int nsamples = (int)samples.getSamples().size();
	printf("%d samples\n", nsamples);

}


void calibrate() {

	int nsamples = (int)samples.getSamples().size();
	if(nsamples >= 2) {

		bool success = calib::calibrateLED(samples, cam);

		if(success) {

			double *LEDEstim = samples.LED_pos;

			printf(
				"Calibration ready, estimated LED position (in OpenCV camera coord. system):\n"
                "\t(%.*f, %.*f, %.*f)\n",
                DBL_PRINT_ACCURACY, LEDEstim[0],
                DBL_PRINT_ACCURACY, LEDEstim[1],
                DBL_PRINT_ACCURACY, LEDEstim[2]);

			/*****************************************************************
			 * Compute the LED position in the camera coord. system
			 *****************************************************************/
			const double *trCam = cameraWidget.getTransMat();
			double invTrCam[16];
			bool b_resInvert = invert(trCam, invTrCam);

			if(b_resInvert) {

				const double *trLED = ledWidget.getTransMat();
				const double vLED[4] = {trLED[12], trLED[13], trLED[14], 1.0};
				double vLEDCam[4];

				multMatVec(invTrCam, vLED, vLEDCam);

				// convert into OpenCV coord. system
				vLEDCam[0] =  vLEDCam[0]; // Wau!
				vLEDCam[1] = -vLEDCam[1]; // invert sign
				vLEDCam[2] = -vLEDCam[2]; // invert sign

				printf(
					"\nReal LED position (in OpenCV camera coord. system):\n"
					"\t(%.*f, %.*f, %.*f)\n",
                     DBL_PRINT_ACCURACY, vLEDCam[0],
                     DBL_PRINT_ACCURACY, vLEDCam[1],
                     DBL_PRINT_ACCURACY, vLEDCam[2]
				);

				double diffX = vLEDCam[0] - LEDEstim[0];
				double diffY = vLEDCam[1] - LEDEstim[1];
				double diffZ = vLEDCam[2] - LEDEstim[2];

				double errX = diffX * diffX;
				double errY = diffY * diffY;
				double errZ = diffZ * diffZ;

				printf(
					"\nSquared error per direction:\n"
					"\t(%.5f, %.5f, %.5f)\n", errX, errY, errZ
				);


				double errTot = sqrt(errX + errY + errZ);

				printf(
					"Total error:\n"
					"\t%.5f\n", errTot
				);

			}
			else {
				printf("Could not compute the real LED position.\n");
			}

		}
		else {



		}
	}
	else {
		printf("you need at least 2 samples, you have %d\n", nsamples);
	} 


}


void cvVecToGlVec(const double cvVec[3], double glVec[3]) {

	/*
	 * Both coordinate systems, OpenCV and OpenGL, are
	 * right-handed. The other one is just rotated 180 degrees
	 * around the x-axis.
	 *
	 * 1         0         0         0
	 * 0         cos(pi)  -sin(pi)   0
	 * 0         sin(pi)   cos(pi)   0
	 * 0         0         0         1
	 *
	 * Becomes
	 *
	 * 1         0         0         0
	 * 0        -1         0         0
	 * 0         0        -1         0
	 * 0         0         0         1
	 */

	glVec[0] =  cvVec[0];
	glVec[1] = -cvVec[1];
	glVec[2] = -cvVec[2];

}

