#include <sys/time.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <math.h>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "GLDrawable.h"
#include "matrix.h"
#include "Camera.h"
#include "LEDCalibrator.h"
#include "LEDCalibPattern.h"
#include "CalibDataReader.h"
#include "GrSamples.h"
#include "InputParser.h"
#include "CalibDataWriter.h"


static const int WIDTH					= 640;
static const int HEIGHT					= 480;
static const double SPEED				= 0.001;
static const double SPEED_ROT			= 0.7;
static const double DEFAULT_PATTERN_Z	= -2;
static const int DBL_PRINT_ACCURACY     = 8;

static bool bCalibrateFirst = false;
static bool bSaveNewlyCalibrated = false;


enum RenderMode {

    RENDER_SAMPLES,
    RENDER_LEDS

};


enum Modifiers {

    KEY_LSHIFT = 1,
    KEY_RSHIFT = 2,
    KEY_LCTRL  = 4,
    KEY_RCTRL  = 8,
    KEY_LALT   = 16,
    KEY_RALT   = 32

};


/**********************************************************
 * Globals
 **********************************************************/

void incContainer(int inc);
void nextContainer();
void prevContainer();

void incSample(int inc);
void nextSample();
void prevSample();

void printUsageInfo();
void printControlInfo();
bool handleInputParameters(int  argc, const char **args);
bool handleParameter(const ParamAndValue &pair);
bool fileExists(const char *filename);



std::string inputFile;



RenderMode renderMode = RENDER_SAMPLES;


GrSamples grSamples;
Camera cam;

// the sample container
std::vector<calib::LEDCalibContainer> calibContainers;

// the current sample
int indCurContainer = 0;
int indCurSample    = 0;

// The upper and lower view
View viewScene;		// the 3D scene

// viewer transformation, column-major
double viewerMat[16] = {
    1.0, 0.0, 0.0, 0.0,     // col 1
    0.0, 1.0, 0.0, 0.0,     // col 2
    0.0, 0.0, 1.0, 0.0,     // col 3
    0.0, 0.0, 0.0, 1.0,     // col 4
};


/**********************************************************
 * Prototypes
 **********************************************************/
bool chooseFirstValidContainer();

void draw();
void drawSample();
void drawLEDs();


void initGL();
bool init_all(SDL_Surface **screen);
bool initSDL(SDL_Surface **screen);
int handle_events();
void main_loop();
void handleDebouncableAction(const unsigned char *keystate, int modifiers);
void printContainerInfo();

bool readCalibData(const std::string &calibFile,
                   calib::CameraCalibContainer &camContainer,
                   std::vector<calib::LEDCalibContainer> &LEDContainers);


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

		// upper view
		viewScene.x			= 0;
		viewScene.y			= 0;
		viewScene.w			= WIDTH;
		viewScene.h			= HEIGHT;
		viewScene.b_ortho	= false;
		viewScene.znear		= 0.001;
		viewScene.zfar		= 200.0;

		// ideal
		static const double FOV_y	= 50.0;
		static const double cx		= (WIDTH - 1.0) / 2.0;
		static const double cy		= (HEIGHT - 1.0) / 2.0;
		static const double fy		= (HEIGHT/2.0) / tan(DEGTORAD(FOV_y / 2.0));
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

    }


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


int main(int argc, const char **argv) {

    /*
     * Must have at least the input file
     */
    if(argc < 2) {

        printUsageInfo();

        return -1;

    }


    /*
     * Handle cmd arguments
     */
    if(!handleInputParameters(argc, argv)) {

        printUsageInfo();

        return -1;

    }


    /*
     * Input file must be defined
     */

    if(inputFile.empty()) {

        printf("Must specify the input file\n");
        printUsageInfo();

        return -1;
    }

    calib::CameraCalibContainer camContainer;
    if(!readCalibData(inputFile.c_str(), camContainer, calibContainers)) {
        printf("Could not read %s\n", inputFile.c_str());
        return -1;
    }

    if(!chooseFirstValidContainer()) {
        printf("No valid containers\n");
        return -1;
    }


    if(bCalibrateFirst) {

        printf("calibrating the camera...");
        fflush(stdout);
        calib::CameraCalibrator::calibrateCamera(camContainer);
        printf("ok\n");

    }

    // set the parameters
    cam.setIntrinsicMatrix(camContainer.intr);
    cam.setDistortion(camContainer.dist);

    if(bCalibrateFirst) {

        printf("calibrating the LEDs\n");

        for(int i = 0; i < (int)calibContainers.size(); ++i) {

            // calibrate
            calib::calibrateLED(calibContainers[i], cam);

        }

    }

    grSamples.reset(calibContainers[0],
                    calibContainers[0].getSamples()[0],
                    cam);


	SDL_Surface *screen = NULL;

	if(!init_all(&screen)) {
		SDL_FreeSurface(screen);
		quit();

		return -1;
	}

	// run the main loop
	main_loop();

    if(bSaveNewlyCalibrated && bCalibrateFirst) {

        std::string input;
        printf(
               "************************************************************\n"
               "You calibrated the results first because you used option -c\n"
               "and you also used -s, i.e. overwrite\n"
               "Would you really like to overwrite %s (yes/no)\n"
               "************************************************************\n",
               inputFile.c_str()
               );

        bool bSave = false;

        while(input != "yes" && input != "no") {

            std::getline(std::cin, input);

            if(input == "no") {

                printf("Not overwriting %s\n", inputFile.c_str());

                bSave = false;

                break;

            }
            else if(input == "yes") {

                printf("overwriting %s\n", inputFile.c_str());

                bSave = true;

                break;

            }

        }

        if(bSave) {

            CalibDataWriter writer;
            if(!writer.create(inputFile.c_str())) {
                printf("Could not create %s\n", inputFile.c_str());
                return -1;

            }

            if(!writer.writeCameraData(camContainer)) {
                printf("Could not write the camera data into the xml file\n");
                return -1;
            }
            if(!writer.writeLEDData(calibContainers)) {
                printf("Could not write the LED data into the xml file\n");
                return -1;
            }

            printf("XML file %s written\n", inputFile.c_str());

        }


    }


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

	// select the scene view
	View::selectView(viewScene);

    glLoadMatrixd(viewerMat);

    switch(renderMode) {


        case RENDER_SAMPLES: {

            drawSample();

            break;

        }


        case RENDER_LEDS: {

            drawLEDs();

            break;

        }

    }

}


void drawSample() {

    grSamples.draw();

}


void drawLEDs() {

    int sz = calibContainers.size();

    std::vector<cv::Point3d> vertices(sz);

    for(int i = 0; i < sz; ++i) {

        const calib::LEDCalibContainer &curContainer = calibContainers[i];

        double oglLEDX =  curContainer.LED_pos[0];
        double oglLEDY = -curContainer.LED_pos[1];
        double oglLEDZ = -curContainer.LED_pos[2];

        GrSamples::drawLED(oglLEDX, oglLEDY, oglLEDZ);


        vertices[i].x = oglLEDX;
        vertices[i].y = oglLEDY;
        vertices[i].z = oglLEDZ;

    }

    if(sz > 1) {

        glBegin(GL_LINE_LOOP);

        for(int i = 0; i < sz; ++i) {
            glVertex3d(vertices[i].x, vertices[i].y, vertices[i].z);
        }

        glEnd();
    }

}


int handle_events() {

	SDL_Event evt;

	int running = 1;

    int modifiers = 0;


	while(SDL_PollEvent(&evt)) {

		switch(evt.type){

			case SDL_KEYDOWN: {

				if(evt.key.keysym.sym == SDLK_ESCAPE) {
					running = 0;
				}

                modifiers |= (evt.key.keysym.mod & KMOD_LSHIFT) ? KEY_LSHIFT : 0;
                modifiers |= (evt.key.keysym.mod & KMOD_RSHIFT) ? KEY_RSHIFT : 0;
                modifiers |= (evt.key.keysym.mod & KMOD_LCTRL)  ? KEY_LCTRL  : 0;
                modifiers |= (evt.key.keysym.mod & KMOD_RCTRL)  ? KEY_RCTRL  : 0;
                modifiers |= (evt.key.keysym.mod & KMOD_LALT)   ? KEY_LALT   : 0;
                modifiers |= (evt.key.keysym.mod & KMOD_RALT)   ? KEY_RALT   : 0;

                // if(evt.key.keysym.mod & KMOD_RCTRL) {
                //     printf("L\n");
                // }

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

        viewerMat[13] -= SPEED;

	}
	else if(keystate[SDLK_x]) {

        viewerMat[13] += SPEED;

	}
	if(keystate[SDLK_UP]) {

        viewerMat[14] += SPEED;

	}
	else if(keystate[SDLK_DOWN]) {

        viewerMat[14] -= SPEED;

	}
	if(keystate[SDLK_RIGHT]) {

        viewerMat[12] -= SPEED;

	}
	else if(keystate[SDLK_LEFT]) {

        viewerMat[12] += SPEED;

	}
	if(keystate[SDLK_s]) {

        double tr[16];
        makeXRotMat(tr, SPEED_ROT);
        multMatSafe(tr, viewerMat, viewerMat);

	}
	else if(keystate[SDLK_w]) {

        double tr[16];
        makeXRotMat(tr, -SPEED_ROT);
        multMatSafe(tr, viewerMat, viewerMat);

	}
	if(keystate[SDLK_a]) {

        double tr[16];
        makeYRotMat(tr, -SPEED_ROT);
        multMatSafe(tr, viewerMat, viewerMat);

	}
	else if(keystate[SDLK_d]) {

        double tr[16];
        makeYRotMat(tr, SPEED_ROT);
        multMatSafe(tr, viewerMat, viewerMat);

	}
	if(keystate[SDLK_q]) {

        double tr[16];
        makeZRotMat(tr, -SPEED_ROT);
        multMatSafe(tr, viewerMat, viewerMat);


	}
	else if(keystate[SDLK_e]) {

        double tr[16];
        makeZRotMat(tr, SPEED_ROT);
        multMatSafe(tr, viewerMat, viewerMat);

	}

	if(keystate[SDLK_r]) {

        setIdentity(viewerMat);

	}


	bool b_debouncable = keystate[SDLK_p] || keystate[SDLK_n] || keystate[SDLK_m] || keystate[SDLK_c];


	if(b_debouncable) {
		handleDebouncableAction(keystate, modifiers);
	}

	return running;

}


void handleDebouncableAction(const unsigned char *keystate, int modifiers) {

	static struct timeval t1 = (struct timeval){0};
	struct timeval t2;
	gettimeofday(&t2, NULL);

	long sec	= t2.tv_sec - t1.tv_sec;
	long usec	= t2.tv_usec - t1.tv_usec;

	double millis = sec * 1000.0 + usec / 1000.0;

	if(millis > 300) {

        bool bShiftDown = modifiers & KEY_LSHIFT || modifiers & KEY_RSHIFT;

        if(keystate[SDLK_n]) {

            bShiftDown ? nextContainer() : nextSample();

        }

        else if(keystate[SDLK_p]) {

            bShiftDown ? prevContainer() : prevSample();

        }

        if(keystate[SDLK_m]) {

            renderMode = renderMode == RENDER_LEDS ? RENDER_SAMPLES : RENDER_LEDS;

        }


		t1 = t2;

	}

}


bool chooseFirstValidContainer() {

    indCurContainer = 0;
    indCurSample = 0;

    int nContainers = calibContainers.size();

    while(indCurContainer < nContainers &&
          calibContainers[indCurContainer].getSamples().size() == 0) {

        ++indCurContainer;

    }

    if(indCurContainer == nContainers) {
        indCurContainer = 0;
        return false;
    }

    printContainerInfo();


    return true;

}


// 1 or -1
void incContainer(int inc) {

    int sz = calibContainers.size();

    int origInd = indCurContainer;

    int ind = indCurContainer + inc;

    if(ind < sz && ind >= 0) {

        indCurSample = 0;

        indCurContainer = ind;

        while(indCurContainer < sz &&
              calibContainers[indCurContainer].getSamples().size() == 0) {

            ++indCurContainer;

            printf("Container %d / %d is empty, selecting %s\n",
                   indCurContainer,
                   (int)sz,
                   inc < 0 ? "previous" : "next");

        }

        if(indCurContainer == sz) {

            indCurContainer = origInd;

            printf("Could not select the %s container\n", inc < 0 ? "previous" : "next");

            return;
        }

        const calib::LEDCalibContainer &curContainer = calibContainers[indCurContainer];
        const calib::LEDCalibSample &curSample       = curContainer.getSamples()[indCurSample];


        printContainerInfo();

        grSamples.reset(curContainer,
                        curSample,
                        cam);

    }

}


void nextContainer() {

    incContainer(1);

}


void prevContainer() {

    incContainer(-1);

}


void incSample(int inc) {

    const calib::LEDCalibContainer &curContainer = calibContainers[indCurContainer];
    int sz = curContainer.getSamples().size();

    int ind = indCurSample + inc;
    if(ind < sz && ind >= 0) {

        indCurSample = ind;

        printf("sample %d / %d\n    %s\n",
               indCurSample + 1,
               sz,
               curContainer.getSamples()[indCurSample].img_path.c_str());

        const calib::LEDCalibSample &curSample = curContainer.getSamples()[indCurSample];

        grSamples.reset(curContainer,
                        curSample,
                        cam);

    }

}


void nextSample() {

    incSample(1);

}


void prevSample() {

    incSample(-1);

}


bool readCalibData(const std::string &calibFile,
                   calib::CameraCalibContainer &camContainer,
                   std::vector<calib::LEDCalibContainer> &LEDContainers) {

	/**********************************************************************
	 * Open the settings file and read the settings of the calib reader
	 *********************************************************************/
	CalibDataReader calibReader;
	if(!calibReader.create(calibFile)) {

        printf("Could not open %s\n", calibFile.c_str());

		return false;

	}


	/**********************************************************************
	 * Read the camera calibration container and assign the values to
	 * the camera
	 *********************************************************************/

    if(!calibReader.readCameraContainer(camContainer)) {

        printf("Could not read the camera container\n");

        return false;

    }


	/**********************************************************************
	 * Load the LED positions from the file and assign the to the pattern
	 *********************************************************************/

    if(!calibReader.readLEDContainers(LEDContainers)) {

        printf("Could not read the LED containers\n");

        return false;

    }

    if(LEDContainers.size() == 0) {

        printf("Read the LED containers, but there is no data...\n");

        return false;

    }


    return true;

}


void printContainerInfo() {

    printf("****************************\n"
           "* Container %d / %d\n"
           "****************************\n",
           indCurContainer + 1,
           (int)calibContainers.size());

}


void printControlInfo() {

    printf("Controls:\n"
           "    Arrow Up:      move forward\n"
           "    Arrow Down:    move backward\n"
           "    Arrow Right:   move right\n"
           "    Arrow Left:    move left\n"
           "    z key:         move up\n"
           "    x key:         move down\n"
           "    a key:         rotate y left\n"
           "    d key:         rotate y right\n"
           "    w key:         rotate x down\n"
           "    s key:         rotate x up\n"
           "    q key:         rotate z left\n"
           "    e key:         rotate z right\n"
           "    m key:         mode, single LED or all LEDs\n"
           "    n key:         next sample\n"
           "    p key:         previous sample\n"
           "    shift + n key: next LED\n"
           "    shift + p key: previous LED\n"
           );

}


void printUsageInfo() {

    printf("Usage:\n"
           "  ./samples [option arguments]\n"
           "  option arguments:\n"
           "      [-i <input_file>]    Must be defined. xml calibration input file, usually calibration.calib\n"
           "      [-c]                 Calibrate camera and LEDs first\n"
           "      [-s]                 Save the calibration results. Neglected if -c is not defined\n"
           "      [-h]                 Display help\n"
           "      [-help]              Same as -h\n\n"
           );

    printControlInfo();

}


bool handleInputParameters(int  argc, const char **args) {

    std::vector<ParamAndValue> argVec;
    if(!parseInput(argc, args, argVec)) {
        printf("handleInputParameters(): Error parsing input\n");
        return false;
    }


    for(int i = 0; i < (int)argVec.size(); ++i) {

        if(!handleParameter(argVec[i])) {

            printf("-%s %s not defined\n", argVec[i].name.c_str(), argVec[i].value.c_str());
            return false;

        }

    }


    return true;

}


bool handleParameter(const ParamAndValue &pair) {

    if(pair.name == "i") {

        inputFile = pair.value;

    }

    else if(pair.name == "s") {

        bSaveNewlyCalibrated = true;

    }

    else if(pair.name == "c") {

        bCalibrateFirst = true;

    }

    else {
        return false;
    }


    return true;

}


bool fileExists(const char *filename) {

    std::ifstream file(filename);
    return file.is_open();

}

