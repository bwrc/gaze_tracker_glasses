#include <GL/glew.h>
#include <SDL/SDL.h>
#include "GLVideoCanvas.h"
#include "BufferWidget.h"
#include "DualFrameReceiver.h"
#include "Settings.h"
#include "SettingsPanel.h"
#include "PanelIdle.h"
#include "trackerSettings.h"
#include <ctime>
#include <sys/time.h>
#include "VideoSync.h"
#include "Timing.h"
#include "GLCornea.h"


/* Extern the global tracker protecting mutex */
extern pthread_mutex_t mutex_tracker;


/*********************************************************
 * Prototypes
 *********************************************************/
static void main_loop();
static void handleEvents();
static bool build_GUI(SettingsIO &settings);
static void collectFramesAndDrawGUI();
static void draw_GUI(CameraFrame *img_eye, CameraFrame *img_scene, const ResultData *res);
static void drawResults(CameraFrameExtended *imgEye, CameraFrameExtended *imgScene, OutputData *data);
static bool init_all(const char *input_file);
static bool init_SDL(SDL_Surface **screen);
static bool init_video(const Settings &settings);
static void quit();
static void print_usage_info();
static void drawRect();
static void drawCorneaSphere(const ResultData *res);

/*********************************************************
 * Static variables
 *********************************************************/

/*
 * GUI related
 *  ______________________________
 *  |     Cam1     |     Cam2     |
 *  -------------------------------
 *  |  Status bars |   Settings   |
 *  -------------------------------
 */
static const int W_FRAME1					= 640;
static const int H_FRAME1					= 480;
static const int W_FRAME2					= 640;
static const int H_FRAME2					= 480;
static const int W_STATUS_BAR				= W_FRAME1;
static const int H_STATUS_BAR				= 200;

static const int BORDER_W					= 2;
static const int WIN_W						= W_FRAME1 + W_FRAME2 + 3 * BORDER_W;
static const int WIN_H						= std::max(H_FRAME1, H_FRAME2) + H_STATUS_BAR + 3 * BORDER_W;
static gui::GLVideoCanvas *panel_eye		= NULL;
static gui::GLVideoCanvas *panel_scene		= NULL;
static gui::BufferWidget *panel_statusbars	= NULL;
static gui::SettingsPanel *panel_settings	= NULL;
static gui::PanelIdle *panelIdle			= NULL;

/* Stream related */
static const int NDEVS						= 2;
static const int FRAMERATE					= 30;
static DualFrameReceiver *receiver			= NULL;
static VideoSync *videoSync                 = NULL;
static bool bCollectForGUI							= true;

/* Other, program state etc. */
static bool b_running						= true;
static gui::GLCornea *pGLCornea             = NULL;
static bool bDrawCornea                     = false;


cv::Point3d lastValidCorneaCentre;

static const double FPSPeriodMs             = 1000.0 / 60.0;

/*
 * An array containing N_POINTS last scene points. This class
 * will automatically remove the oldest point once a new sample
 * is added.
 */
class GazeArray {

public:

    /* Max number of points in the list */
    enum Limit {
        MAX_N_POINTS = 15
    };

    GazeArray() {
        m_nIndCurr = 0;
    }

    /*
     * Adds a new point replacing the oldest.
     */
    void add(const cv::Point2d &p) {

        /*
         * if the buffer is not full, push back else replace oldest
         */
        if(m_vecPoints.size() < MAX_N_POINTS) {
            m_vecPoints.push_back(p);
        }
        else {
            m_vecPoints[m_nIndCurr] = p;
        }

        m_nIndCurr = (m_nIndCurr + 1) % MAX_N_POINTS;

    }

    /*
     * Removes the oldest sample
     */
    void removeOldest() {

        // cannot remove from an empty vector
        if(m_vecPoints.size() == 0) {return;}

        // index of oldest
        int nIndOldest = m_nIndCurr != 0 ? m_nIndCurr - 1 : m_vecPoints.size() - 1;

        // iterator to the oldest
        std::vector<cv::Point2d>::iterator itOldest = m_vecPoints.begin() + nIndOldest;

        // erase the oldest
        m_vecPoints.erase(itOldest);

        // if we have an empty vector
        if(m_vecPoints.size() == 0) {
            m_nIndCurr = 0;
        }
        else {
            m_nIndCurr = m_nIndCurr != 0 ? m_nIndCurr - 1 : m_vecPoints.size() - 1;
        }

    }

    const std::vector<cv::Point2d> &getVector() const {
        return m_vecPoints;
    }

private:

    /*
     * Vector holding the points. Circular buffer
     */
    std::vector<cv::Point2d> m_vecPoints;

    /*
     * Index pointing where the next point should be inserted.
     */
    unsigned int m_nIndCurr;

};

GazeArray scenePointArray;



int main(int nof_args, const char **args) {

    /******************************************
     * check args
     ******************************************/
    if(nof_args < 2) {

        print_usage_info();

        return EXIT_FAILURE;
    }

    /******************************************
     * initialise everything
     ******************************************/
    if(!init_all(args[1])) {

        quit();

        return EXIT_FAILURE;
    }


    /******************************************
     * run the main loop
     ******************************************/
    main_loop();


    /******************************************
     * clean up
     ******************************************/
    quit();


    /******************************************
     * exit the main thread
     ******************************************/
    pthread_exit(NULL);


    /******************************************
     * return success
     ******************************************/
    return EXIT_SUCCESS;

}


void main_loop() {

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT);


    /***********************************************************
     *  The main loop
     ***********************************************************/
    while(b_running) {

        utils::Timing timingFps;

        // handle the keyboard presses
        handleEvents();


        /***********************************************************
         * Collect frames for the GUI or not
         ***********************************************************/
        if(bCollectForGUI) {

            collectFramesAndDrawGUI();

        }
        else {

            drawRect();

        }

        double millis = timingFps.getElapsedMicros() * 0.001;
        if(millis < FPSPeriodMs) {
            Uint32 delayMs = FPSPeriodMs - millis;
            SDL_Delay(delayMs);
        }

        // display contents
        SDL_GL_SwapBuffers();

    }

}


void handleEvents() {

    /********************************************************************
     * See if the window was closed by pressing the cross. We Also pump
     * the event loop, which in turn, in addition to many other things,
     * updates the keystate.
     ********************************************************************/
    SDL_Event evt;

    while(SDL_PollEvent(&evt)) {

        switch(evt.type) {

            case SDL_QUIT: {
                b_running = false;
                return;
            }

            case SDL_KEYDOWN: {

                if(evt.key.keysym.sym == SDLK_b) {
                    bDrawCornea = !bDrawCornea;
                }
                else if(evt.key.keysym.sym == SDLK_c) {
                    // clear the screen before painting
                    glClear(GL_COLOR_BUFFER_BIT);

                    bCollectForGUI = !bCollectForGUI;
                    receiver->collectForGUI(bCollectForGUI);
                }
                else if(evt.key.keysym.sym == SDLK_ESCAPE) {

                    b_running = false;

                }

                break;
            }
            default: break;

        }

    }


    /****************************************************************
     * now get the current key state
     ****************************************************************/
    const unsigned char *keystate = SDL_GetKeyState(NULL);

    // update settings only when the GUI frame collection is enabled
    if(bCollectForGUI) {

        /* The settings panel is the only keyboard listener */
        bool b_settings_changed = panel_settings->handleKeys(keystate);

        /* Assing the new settings only when requested */
        if(b_settings_changed) {

            /* get a pointer to the panel's settings */
            LocalTrackerSettings *lts = panel_settings->getSettings();

            // protect the tracker
            pthread_mutex_lock(&mutex_tracker);

            // set new settings
            trackerSettings.set(*lts);

            pthread_mutex_unlock(&mutex_tracker);

        }

    }

    /****************************************************************
     * quit if ESC was pressed
     ****************************************************************/
    // if(keystate[SDLK_ESCAPE]) {
    // 	b_running = false;
    // }

    // else if(keystate[SDLK_c]) {

    // 	static struct timeval tThen = (struct timeval){0};

    // 	struct timeval tNow;
    // 	gettimeofday(&tNow, NULL);
    // 	long dSec = tNow.tv_sec - tThen.tv_sec;
    // 	long dUsec = tNow.tv_usec - tThen.tv_usec;

    // 	double dMillis = 1000.0 * dSec + dUsec / 1000.0;

    // 	// debounce
    // 	if(dMillis > 500.0) {

    // 		// clear the screen before painting
    // 		glClear(GL_COLOR_BUFFER_BIT);

    // 		bCollectForGUI = !bCollectForGUI;
    // 		receiver->collectForGUI(bCollectForGUI);

    // 		tThen = tNow;

    // 	}

    // }

}


bool init_all(const char *input_file) {

    /***********************************************************
     * Init gstreamer
     ***********************************************************/
    gst_init(0, NULL);


    /***********************************************************
     * Read the settings
     ***********************************************************/
    Settings settings;
    if(!settings.readSettings(input_file)) {

        printf("main(): Could not read settings\n");

        return false;
    }


    /**************************************************
     * Apply the tracker settings
     **************************************************/
    SettingsIO settingsFile(settings.gazetrackerFile);
    LocalTrackerSettings localSettings;
    localSettings.open(settingsFile);
    trackerSettings.set(localSettings);


    /***********************************************************
     * Initialise SDL and create the window
     ***********************************************************/
    SDL_Surface *screen = NULL;

    if(!init_SDL(&screen)) {

        printf("main(): Unable to initialize SDL: %s\n", SDL_GetError());

        return false;

    }


    glewInit();


    /***********************************************************
     * Initialise the video streams
     ***********************************************************/
    if(!init_video(settings)) {

        return false;
    }


    /***********************************************************
     * Compute the panel dimensions etc.
     ***********************************************************/
    if(!build_GUI(settingsFile)) {
        printf("main(): Could not build GUI\n");

        return false;
    }

    return true;

}


bool init_SDL(SDL_Surface **screen) {

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("main(): Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0); // no v-sync

    *screen = SDL_SetVideoMode(WIN_W, WIN_H, 24, SDL_OPENGL);

    if((*screen) == NULL) {
        printf("main(): screen is NULL\n");
        return false;
    }

    glDisable(GL_DEPTH_TEST);
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_FASTEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);



    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    return true;

}


bool init_video(const Settings &settings) {

    // the parent directory inside which the result directory will be created
    const char *oput_parent_dir = settings.oput_dir.c_str();

    /*
     * It is important to create receiver before videos, because the receiver
     * must be ready when the cameras start streaming
     */
    receiver = new DualFrameReceiver(bCollectForGUI);
    bool bOnlyResults = true;
    if(strstr(settings.dev1.c_str(), "/dev/video") != NULL) {
        bOnlyResults = false;
    }

    if(!receiver->init(bOnlyResults,
                       oput_parent_dir,
                       settings.eyeCamCalibFile,
                       settings.sceneCamCalibFile,
                       settings.mapperFile)) {

        return false;

    }


    // create the video information containers
    std::vector<VideoInfo> info(NDEVS);

    // Video 1
    info[0].devname	= settings.dev1;
    info[0].w		= W_FRAME1;
    info[0].h		= H_FRAME1;
    info[0].fps		= FRAMERATE;
    info[0].format	= FORMAT_MJPG;

    // Video 2
    info[1].devname	= settings.dev2;
    info[1].w		= W_FRAME2;
    info[1].h		= H_FRAME2;
    info[1].fps		= FRAMERATE;
    info[1].format	= FORMAT_MJPG;

    videoSync = new VideoSync();
    if(!videoSync->init(info, receiver)) {
        std::cout << "main(): Could not initialise the video sync object" << std::endl;
        return false;
    }
    videoSync->start();


    return true;

}


void print_usage_info() {

    printf(
           "Usage:\n"
           "  ./gazetoworld <settingsfile>.xml\n"
           );

}


void collectFramesAndDrawGUI() {

    /* Camera frames and tracking results */
    OutputData *data = receiver->getNewestData();

    // try to get the video frames
    if(data != NULL) {

        // array of pointers to the frames
        CameraFrameExtended **frames = data->frames;

        /* pointers to the frames in data */
        CameraFrameExtended *img_eye	= frames[0];
        CameraFrameExtended *img_scene	= frames[1];

        // draw the results
        drawResults(img_eye, img_scene, data);

        // draw the GUI
        draw_GUI(img_eye, img_scene, data->res);

        /*
         * we must delete these images since the VideoHandler donated
         * them to us
         */
        data->releaseFrames();
        delete data;

    }

    /* There were no new frames in the receiver, just update the GUI */
    else {

        // draw the GUI
        draw_GUI(NULL, NULL, NULL);

    }

}


void draw_GUI(CameraFrame *img_eye, CameraFrame *img_scene, const ResultData *res) {

    if(img_eye != NULL && img_scene != NULL) {

        // draw the frames to the GUI
        panel_eye->draw(img_eye);
        panel_scene->draw(img_scene);

    }

    panel_settings->draw();

    size_t worker_buffer_sizes[2];

    receiver->getWorkerBufferStates(worker_buffer_sizes);
    size_t n_video_queue_frames	= receiver->getVideoBufferState();
    int n_save_queue_frames		= receiver->getSaveBufferState();
    size_t max_worker_buffer	= receiver->getMaxBufferSize();

    gui::BufferData data;
    data.max_video_frames	= max_worker_buffer;
    data.max_worker_frames	= max_worker_buffer;
    data.max_save_frames	= max_worker_buffer;
    data.n_video_frames		= n_video_queue_frames;
    data.n_worker_frames1	= worker_buffer_sizes[0];
    data.n_worker_frames2	= worker_buffer_sizes[1];
    data.n_save_frames		= n_save_queue_frames;

    // draw to settings panel to the GUI
    panel_statusbars->draw(data);



    /***************************************************************
     * Draw the cornea sphere if requested
     **************************************************************/
    if(res != NULL && bDrawCornea) {
        drawCorneaSphere(res);
    }


}


void drawResults(CameraFrameExtended *imgEye, CameraFrameExtended *imgScene, OutputData *data) {

    if(!data->res->bTrackSuccessfull) {
        //        return;
    }


    /************************************************************
     * All contours
     ************************************************************/

    // get the contours
    const std::vector<std::vector<cv::Point> > &listContours = data->res->listContours;

    // make an openCV header of the frame
    cv::Mat ocvImgEye(
                      imgEye->h,				// rows
                      imgEye->w,				// cols
                      CV_8UC3,				// type of data
                      imgEye->data,			// data
                      imgEye->w*imgEye->bpp	// bytes per row
                      );

    // flip around y-axis
    cv::flip(ocvImgEye, ocvImgEye, 1);

    if(listContours.size() > 0) {

        cv::drawContours(ocvImgEye,					// opencv image
                         listContours,				// list of contours to be drawn
                         -1,							// draw all contours in the list
                         cv::Scalar(0, 255, 255),	// colour
                         2,							// thickness
                         CV_AA);					// line type

    }


    /************************************************************
     * Pupil ellipse
     ************************************************************/
    const cv::RotatedRect &ellipse_pupil = data->res->ellipsePupil;
    cv::ellipse(ocvImgEye,				// opencv image
                ellipse_pupil,			// pupil ellipse
                cv::Scalar(0, 0, 255),	// colour
                2,						// thickness
                CV_AA);					// line type anti-aliased


    /************************************************************
     * Corneal reflections
     ************************************************************/
    const std::vector<cv::Point2d> &crs = data->res->listGlints;
    if(crs.size()) {
        if(crs[0].x != -1) {
            for(int i = 0; i < (int)crs.size(); ++i) {
                int x = (int)(crs[i].x + 0.5);
                int y = (int)(crs[i].y + 0.5);
                int x1 = x - 5;
                int x2 = x + 5;
                int y1 = y - 5;
                int y2 = y + 5;
                cv::line(ocvImgEye, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
                cv::line(ocvImgEye, cv::Point(x2, y1), cv::Point(x1, y2), cv::Scalar(0, 255, 0), 2);
            }
        }
    }


    /************************************************************
     * Gaze vector
     ************************************************************/
    cv::line(ocvImgEye,
             data->res->gazeVecStartPoint2D,
             data->res->gazeVecEndPoint2D,
             cv::Scalar(0, 0, 255),
             3);


    /***************************************************************
     * Draw the point in the scene frame
     **************************************************************/

    // make an openCV header of the frame
    cv::Mat ocvImgScene(
                        imgScene->h,				// rows
                        imgScene->w,				// cols
                        CV_8UC3,					// type of data
                        imgScene->data,				// data
                        imgScene->w*imgScene->bpp	// bytes per row
                        );

    if(data->res->bTrackSuccessfull) { // add point only when tracking was succesful
        scenePointArray.add(data->res->scenePoint);
    }
    else {
        scenePointArray.removeOldest();
    }

    const std::vector<cv::Point2d> &vec = scenePointArray.getVector();

    cv::Point2d filteredPoint;
    double kerroin = 0;
    int SHOW_FILTERED_POINT = 1;

    for(int i = 0; i < (int)vec.size(); ++i) {

        cv::circle(ocvImgScene,
                   vec[i],
                   10,
                   cv::Scalar(255,0,0),
                   2,
                   CV_AA);
	
        filteredPoint = filteredPoint + (i+1.0)/(vec.size()+1.0)*vec[i];
        kerroin = kerroin + (i+1.0)/(vec.size()+1.0);
    }

    if(data->res->bTrackSuccessfull) {
        filteredPoint = filteredPoint + data->res->scenePoint;
        kerroin = kerroin + 1;
    }
    filteredPoint = filteredPoint * (1.0/(kerroin));
    
    if (SHOW_FILTERED_POINT)
        cv::circle(ocvImgScene, filteredPoint, 20, cv::Scalar(0,255,0), 3, CV_AA);
    cv::circle(ocvImgScene, data->res->scenePoint, 10, cv::Scalar(0,0,255), 2, CV_AA);

}


void drawCorneaSphere(const ResultData *res) {

    pthread_mutex_lock(&mutex_tracker);
    double dRho = trackerSettings.RHO;
    double dRd = trackerSettings.rd;
    pthread_mutex_unlock(&mutex_tracker);

    const Camera *cam = receiver->getEyeCamera();

    pGLCornea->render(res,
                      cam->getIntrisicMatrix(),
                      cam->getDistortion(),
                      dRho, dRd);

}


void drawRect() {

    glClear(GL_COLOR_BUFFER_BIT);
    panelIdle->draw();

}


bool build_GUI(SettingsIO &settings) {

    /*************************************************
     * Camera 1
     *************************************************/
    gui::View view1;
    view1.x = BORDER_W;
    view1.y = H_STATUS_BAR + 2*BORDER_W;
    view1.w = W_FRAME1;
    view1.h = H_FRAME1;
    view1.b_ortho = true;

    panel_eye	= new gui::GLVideoCanvas(view1);


    /*************************************************
     * Camera 2
     *************************************************/
    gui::View view2;
    view2.x = view1.x + view1.w + BORDER_W;
    view2.y = view1.y;
    view2.w = W_FRAME2;
    view2.h = H_FRAME2;
    view2.b_ortho = true;

    panel_scene	= new gui::GLVideoCanvas(view2);


    /*************************************************
     * Status bar panel
     *************************************************/
    gui::View view3;
    view3.x = BORDER_W;
    view3.y = BORDER_W;
    view3.w = W_STATUS_BAR;
    view3.h = H_STATUS_BAR;
    view3.b_ortho = true;

    panel_statusbars = new gui::BufferWidget(view3);


    /*************************************************
     * Settings panel
     *************************************************/
    gui::View view4 = view3;
    view4.x = view3.x + view3.w + 2*BORDER_W;

    panel_settings = new gui::SettingsPanel(view4, settings);



    /*************************************************
     * Camera 2
     *************************************************/
    gui::View view5;
    view5.x = 0;
    view5.y = 0;
    view5.w = WIN_W;
    view5.h = WIN_H;
    view5.b_ortho = true;

    panelIdle = new gui::PanelIdle(view5);


    /*************************************************
     * Cornea visualiser
     *************************************************/
    gui::View viewCornea = view1;
    pGLCornea = new gui::GLCornea(viewCornea);
    const std::vector<cv::Point3d> &vecLEDPositons = receiver->getLEDPositions();

    // OpenCV to OpenGL
    std::vector<cv::Point3d> vecLEDPositonsConverted(vecLEDPositons.size());
    for(size_t i = 0; i < vecLEDPositonsConverted.size(); ++i) {

        cv::Point3d p = vecLEDPositons[i];
        p.y *= -1;
        p.z *= -1;
        vecLEDPositonsConverted[i] = p;

    }

    pGLCornea->setLightPositions(vecLEDPositonsConverted);


    return true;

}


void quit() {

    if(videoSync != NULL) {
        printf("main quit(): end videoSync...");
        fflush(stdout);
        videoSync->end();
        delete videoSync;
        printf("ok\n");
    }


    delete receiver;

    delete panel_eye;
    delete panel_scene;
    delete panel_statusbars;
    delete panel_settings;
    delete panelIdle;
    delete pGLCornea;

    SDL_Quit();

}

