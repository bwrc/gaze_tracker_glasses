#include "RenderThread.h"


RenderThread::RenderThread(RenderHandler *_handler) {
	{
		QMutexLocker locker(&mutex);
		b_running = true;
	}

	handler = _handler;


	memset(&buff, 0, sizeof(RENDER_BUFFER));
	buff.mutex	= new QMutex();
	buff.cond	= new QWaitCondition();


	if(_handler != NULL) {
		connect(this, SIGNAL(do_render()), this, SLOT(on_render()));
	}
}


RenderThread::~RenderThread() {
	delete buff.mutex;
	delete buff.cond;
}


void RenderThread::init(RenderHandler *_handler) {
	handler = _handler;

	if(_handler != NULL) {
		connect(this, SIGNAL(do_render()), this, SLOT(on_render()));
	}
}


/*
 *	This method is invoked by the message queue, as the run() method, running in its own thread, has emitted
 *	a corresponding command. This method calls the handler's render method and acknowledges the call to
 *	this function so the run() method will continue if waiting for the queue to empty.
 */
void RenderThread::on_render() {
	handler->render_command();
	acknowledge();
}


/*
 *	Runs in its own thread. Adds messages to the message queue when it's time to render.
 *	Also this method checks that the renderer can keep up with the loop, waits if necessary.
 */
void RenderThread::run() {
	static int c_wait = 0;

	while(isRunning()) {

		// put a message in the message queue
		emit do_render();

		{
			// lock the mutex
			QMutexLocker locker(buff.mutex);

			// increment the buffer count
			++(buff.count);

			// wait if buffer is already full
			while(buff.count > MAX_BUFF_SIZE && isRunning()) {
				buff.b_waiting = true;
				buff.cond->wait(buff.mutex, 500);

				++c_wait;
				printf("waiting %d\n", c_wait);
			}
		}

		QThread::msleep(200);
	}
}


/*
 *	Called by on_render(). Decreases the buffer count and wakes the possibly waiting run() method.
 */
void RenderThread::acknowledge() {
	QMutexLocker locker(buff.mutex);
	--(buff.count);
	if(buff.b_waiting) {
		buff.cond->wakeOne();
		buff.b_waiting = false;
	}
}


bool RenderThread::isRunning() {
	QMutexLocker locker(&mutex);
	return b_running;
}


void RenderThread::stop() {
	QMutexLocker locker(&mutex);
	b_running = false;
}

