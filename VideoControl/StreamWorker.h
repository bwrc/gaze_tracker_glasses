#ifndef STREAMWORKER_H
#define STREAMWORKER_H


#include <list>
#include "VideoBuffer.h"
#include "CameraFrame.h"
#include <pthread.h>


/*
 * This abstract class defines a callback handler class. Its framProcessed()
 * function will be launched in the run method as the frame has been processed.
 */
class WorkerCBHandler {

	public:

		/*
		 * This function will be launched as soon as the frame was processed.
		 * The return value indicates the owner of the frame:
		 *
		 *     true  : This handler took ownership of the frame
		 *     false : The worker will remain as the owner of the frame
		 */
		virtual bool frameProcessed(CameraFrame *_frame, void *user_data) = 0;


		virtual ~WorkerCBHandler() {}
};


class StreamWorker {

	public:

		StreamWorker();
		virtual ~StreamWorker();

		virtual bool init(WorkerCBHandler *_cb_handler, int _max_n_frames, void *_user_data);

		/*
		 * The thread function. Must not be called although it is public.
		 * This is called by when a thread running this class has been
		 * created.
		 */
		void run();

		bool start();


		/*
		 * Joins the thread.
		 * Must be called before destruction. Not included in the destructor,
		 * because in case of multiple instances of this class, the user,
		 * May want to join all threads. 
		 */
		void end();

		/*
		 * Adds the given frame to the list, if there is room for it.
		 * This class becomes the owner of the given frame and therefore
		 * the caller must not do anything with the frame after calling this
		 * function.
		 */
		void add(CameraFrame *_frame);

		/* Query if there is space in this workers queue */
		bool isSpace();

		/* Return the current size of the queue */
		size_t getBufferState();

	private:

		/* returns the current state. The function is mutex protected */
		bool running();

		/* Get the next frame from the list, the function is mutex protected */
		CameraFrame *getNextFrame();

		/*
		 * A method to be implemented by the child class. Called in the thread,
		 * When a frame is ready to be processed
		 */
		virtual CameraFrame *process(CameraFrame *img_compr) = 0;


		/************************************************
		 * thread
		 ************************************************/

		pthread_t thread;



		/************************************************
		 * list of processable frames
		 ************************************************/

		// protection and sync for the list
		pthread_mutex_t mutex_list;
		pthread_cond_t cond_list;

		// list of processable frames
		std::list<CameraFrame *> frames;

		/* The maximum number of frames in this worker's queue */
		int max_n_frames;


		/************************************************
		 * worker's state
		 ************************************************/

		pthread_mutex_t mutex_running;

		// state of the worker
		volatile bool b_running;


		// user defined data
		void *user_data;


		WorkerCBHandler *cb_handler;


};


#endif

