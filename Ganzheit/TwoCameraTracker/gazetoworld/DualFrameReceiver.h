#ifndef DUALCAMERARECEIVER_H
#define DUALCAMERARECEIVER_H


#include "CameraFrame.h"
#include "GazeTracker.h"
#include "DevNullWriter.h"
#include "VideoWriter.h"
#include "GTWorker.h"
#include "ResultData.h"



/*
 * Extended camera frame with an id and a pointer to the track results.
 */
class CameraFrameExtended : public CameraFrame {

public:

    /* The TrackResults will not be deallocated by this instance. */
    CameraFrameExtended(unsigned long _id,
                        ResultData *_res,
                        int w					= 0,
                        int h					= 0,
                        int bpp					= 0,
                        unsigned char *data		= NULL,
                        size_t sz				= 0,
                        Format format			= FORMAT_RGB,
                        bool b_copy_data		= false,
                        bool b_become_parent	= true)

        : CameraFrame(w, h, bpp, data, sz, format, b_copy_data, b_become_parent) {

        id = _id;
        res = _res;

    }

    CameraFrameExtended(const CameraFrameExtended &orig) : CameraFrame(orig) {
        id = orig.id;
        res = orig.res;
    }


    CameraFrameExtended(const CameraFrame &orig) : CameraFrame(orig) {
        id = 0;
        res = NULL;
    }


    unsigned long id;

    ResultData *res;

};


class OutputData {

public:

    OutputData();

    ~OutputData();

    void releaseFrames();

    CameraFrameExtended **frames;
    ResultData *res;

};




/*
 *     ____________      ____________
 *     | camera 1 |      | camera 2 |
 *     ------------      ------------
 *            \              /
 *             \            /
 *           ___\__________/___
 *           |   VideoSync    |
 *           ------------------
 *                   |
 *                   | frame pair
 *         __________|__________
 *         | DualFrameReceiver |
 *         ---------------------
 *          |        |        |
 *  frame 1 |        |        | frame 2
 *  ________|___     |  ______|_____
 *  | worker   |     |  | worker   |
 *  | thread 1 |     |  | thread 2 |
 *  ------------     |  ------------
 *          |  \     |        |
 *          |   \res |        |
 *          |   _\ __|_____   |
 *          |   | saver  |    |
 *          |   | thread |    |
 *          |   ----------    |
 *           \               /
 *          __\_____________/_
 *          | pair collector |
 *          ------------------
 *                  |
 *               ___|___
 *               | GUI |
 *               -------
 *
 */
class DualFrameReceiver : public WorkerCBHandler {

public:

    DualFrameReceiver(bool _bGUIActive);
    ~DualFrameReceiver();

    /*
     * Initialise everything. Must be called only once.
     * saveDir must contain the trailing '/'.
     */
    bool init(bool bOnlyResults,
              const std::string &saveDir,
              const std::string &settings_file,
              const std::string &sceneCamCalibFile,
              const std::string &mapperFile);


    /*
     * Protected by a mutex. Execution must be fast.
     * The highest priority is to place the frames to
     * the saver queue.
     */
    void framesReceived(const CameraFrame *_frameEye, const CameraFrame *_frameScene);


    /*
     * Get the oldest frame pair, succeeds if both frames are ready.
     * Important! The caller must release the images with realaseFrames()
     * and "delete".
     */
    OutputData *getData();


    /*
     * Get the newest frame pair, succeeds if both frames are ready.
     * Destroys all pairs that are older.
     * Important! The caller must release the images with realaseFrames()
     * and "delete".
     */
    OutputData *getNewestData();


    /*
     * Inherited from WorkerCBHandler, called by the workers and must
     * therefore be mutex protected. This function synchronises the
     * GUI frame pairs if the GUI is active. Execution must be fast
     * because the GUI is at a lower priority than the workers.
     */
    bool frameProcessed(CameraFrame *_frame, void *user_data);

    /* Return the current buffer queue sizes */
    void getWorkerBufferStates(size_t states[2]);

    /* Return the maximum allowed worker buffer queue size */
    int getMaxBufferSize();

    /* Return the number of frames ready for the GUI */
    int getVideoBufferState();

    /* Return the number of frames in the saver's queue */
    int getSaveBufferState();

    /* Return the eye camera */
    const Camera *getEyeCamera() const {return camEye;}

    /* Return the scene camera */
    const Camera *getSceneCamera() const {return camScene;}

    /* Return the LED position vector  */
    const std::vector<cv::Point3d> getLEDPositions() {return m_vecLEDPositions;}

    /* Return a pointer to the tracker */
    const gt::GazeTracker *getTracker() const {return tracker;}


    /* Whether or not to collect frames for the GUI. */
    void collectForGUI(bool bCollect);

private:

    /* Create the gaze tracker */
    bool createTracker(const std::string &eyeCamCalibFile,
                       const std::string &sceneCamCalibFile,
                       const std::string &mapperFile);

    /* Configure the camera data. Called by createTracker() */
    bool configureEyeCamData(const std::string &eyeCamCalibFile);
    bool configureSceneCamData(const std::string &sceneCamCalibFile);
    bool configureMapper(const std::string &file);


    /* Join and dstroy the worker threads */
    void destroyWorkers();

    /* A function that tells an instance of this class is alive */
    bool alive();

    /*
     * Save the results to the output file. This function is called by
     * frameProcessed(), which in turn is called from the GTWorker's thread.
     */
    void saveResults(OutputData *oput);


    /* Protect the class state */
    pthread_mutex_t mutex_alive;
    pthread_cond_t cond_alive;
    volatile bool b_alive;

    /* for each stream there is a worker to decode the data etc. */
    std::vector<StreamWorker *> workers;

    /* User defined data for the workers, stored in this vector */
    std::vector<int *> list_worker_user_data;

    /* Output video writer */
    DataWriter *video_writer;

    pthread_mutex_t mutex_receive;

    /* A mutex protecting the output frames */
    pthread_mutex_t mutex_oput;

    /* This condition variable is used for synchronising the requests for the output list */
    pthread_cond_t cond_oput;

    /* The gaze tracker for one of the workers */
    gt::GazeTracker *tracker;

    /* Maps points to the scene */
    SceneMapper *mapper; 

    /* Camera for the tracker */
    Camera *camEye;

    /* Camera for the scene */
    Camera *camScene;

    /* List of tracking results */
    std::list<OutputData *> list_oput;


    /* Count the number of frame pairs received */
    unsigned long n_received_pairs;

    /* Tells if the workers are running, true if init was successful */
    bool b_workers_running;

    /*
     * The mutex _must_ be locked and released manually, i.e. those
     * operations will _not_ be performed in this function
     */
    void flushGUIQueue();

    /* Tells if the receiver is collecting GUI frames or not. */
    bool isGUIActive();

    /*
     * This variable indicates whether to process both frames for drawing
     * to the GUI, or just to analyse the eye frame. If enabled, the scene
     * camera frame will also be decompressed, which is computationally
     * expensive. Therefore this feature should only be enabled when the
     * images are really being observed.
     */
    bool bGUIActive;

    /*
     * LED positions read from the file.
     */
    std::vector<cv::Point3d> m_vecLEDPositions;

};


#endif

