#ifndef VIDEOSTREAMER_H
#define VIDEOSTREAMER_H


#include <opencv2/highgui/highgui.hpp>
#include <QWidget>
#include <QThread>
#include <qmutex.h>
#include <QMutexLocker>
#include <QWaitCondition>
#include "streamer_structs.h"
#include "FrameBuffer.h"



class StreamWorker;
class SettingsStreamer;
class StreamHandler;


typedef struct CAMERA_STATE {
	bool isPaused;
	bool isEnded;
	bool initOK;

	QMutex *mutex;

	QMutex *mutex_sleeper;
	QWaitCondition *cond_sleeper;

	// the position of the frame
	int frame_pos;

	// the total number of frames
	int tot_frames;
} CAMERA_STATE;


/* A class for capturing and processing data from the camera */
class VideoStreamer : public QThread {
	Q_OBJECT

	public:
		VideoStreamer(StreamHandler *handler);
		virtual ~VideoStreamer();


		int getBuffLen() {return 0;}

		/*
		 *	Go to the specified frame
		 */
		void setFrame(unsigned int frame);


		/*
		 *	Set the video size, works only if in camera mode
		 */
		void setVideoDim(int w, int h);


		/*
		 *	Skip n frames. n may be either positive or negative.
		 */
		void skipFrames(int n);

		void startCapture();
		void pauseCapture();
		void stopCapture();

		/* Gets a pointer to the camera settings */
		SettingsStreamer *getSettings() {return this->settings;}

		/* subclasses may override this function in order to do something with the frame */
		virtual void doStuffWithFrame(cv::Mat &img);

	public slots:
		void onFrameChanged(STREAM_FRAME_CHANGED_DATA *);

	protected:
		/*
		 *	Called only in the run()-method's thread.
		 */
		virtual bool init();


		StreamHandler *handler;

		SettingsStreamer *settings;

		/* The capture object returns an image in the BGR form */
		cv::Mat img_bgr;

	private:
		/*
		 *	inherited from QtThread
		 */
		virtual void run();


		/*
		 *	Called only in the run()-method's thread at the  beginning of each iteration.
		 *	It checks if a call to any of the methods changing the camera settings have
		 *	been called and applies those changes. It is important that this thread is
		 *	responsible for making the actual changes according to the other thread's
		 *	request.
		 */
		void applyChanges();


		/*
		 *	Called only in the run()-method's thread.
		 */
		void doChangeInput();

		/*
		 *	Called only in the run()-method's thread.
		 */
		void doSetFrame(int new_pos);


		/*
		 *	Called only in the run()-method's thread.
		 */
		void doChangeVideoSize();


		/*
		 *	Called only in the run()-method's thread.
		 */
		bool readFrame();

		/*
		 *	Called only in the run()-method's thread.
		 */
		void fillQueue();


		/*
		 *	Accessed only in the run()-method's thread.
		 */
		FrameBuffer frames;

		/* The capture object for obtaining data from the camera */
		cv::VideoCapture capture;

		CAMERA_STATE cam_state;

		std::vector<STREAM_INIT_DATA *> list_init_data;

		signals:
			void streamInit(STREAM_INIT_DATA *init_data);
};



class StreamWorker : public QThread {
	Q_OBJECT

	public:
		StreamWorker(	CAMERA_STATE *cam_state,
						FrameBuffer *frames,
						SettingsStreamer *settings,
						VideoStreamer *cam);

		~StreamWorker();


		/* inherited from wxThread */
		virtual void run();
	private:
bool waitGUI(int millis);
		StreamHandler *handler;

//		QMutex mutexGUI;
//		QWaitCondition condGUI;

//		pthread_mutex_t *mutexGUI;
//		pthread_cond_t *condGUI;

		cv::Mat imgGUI;

		FrameBuffer *frames;

		VideoStreamer *streamer;
		SettingsStreamer *settings;
		CAMERA_STATE *cam_state;

		/*
		 *	The GUI might not be able to acknowledge the frameChanged() -signal within the given
		 *	time interval, so the STREAM_FRAME_CHANGED_DATA struct will be put into this vector
		 *	and deallocated as the GUI acknowledges the next message, or as the program ends. At
		 *	this point we know that the old message has been lost and we can deallocate the old 
		 *	messages.
		 */
		std::list<STREAM_FRAME_CHANGED_DATA *> vec_frame_data;

		signals:
			void frameChanged(STREAM_FRAME_CHANGED_DATA *data);
};



class SettingsStreamer {
	public:
		SettingsStreamer();
		~SettingsStreamer();

		// sets
		void setFps(int fps);
		void setSize(int w, int h);
		void setFrame(int frame);
		void setInput(const char *fName);

		// gets
		int getFps();
		void getSize(int *w, int *h);
		int getWidth();
		int getHeight();
		void getInput(char *fName);
		int getFrame();

		void resetChanges();
		unsigned char getChanges();
	private:
		/* A struct containing the camera settings */
		typedef struct SETTINGS_STREAMER {
			int fps;
			int width;
			int height;
			char fName[255];
			int frame_pos;
		} SETTINGS_STREAMER;


		/*
			bit 1:	MODE_IDLE
			bit 2:	MODE_CALIBRATION
			bit 3:	MODE_TRACKING
			bit 4:	input changed
			bit 5:	frame position changed
			bit 6:	size changed
			bit 7:	unused
			bit 8:	unused
		*/
		unsigned char requested_changes;

		SETTINGS_STREAMER settings;
		QMutex *mutex;
};



/* Must extend QWidget, so that signals and slots work */
class StreamHandler : public QWidget {
	Q_OBJECT

	public:
		StreamHandler(QWidget *parent = 0) : QWidget(parent) {}
		virtual void onFrameChanged(STREAM_FRAME_CHANGED_DATA *data) = 0;
		VideoStreamer *streamer;
		QMutex streamerLocker_mutex;

	public slots:
		virtual void onStreamInit(STREAM_INIT_DATA *init_data) = 0;

};



#endif

