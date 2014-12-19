#ifndef RENDER_THREAD_H
#define RENDER_THREAD_H


#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QtOpenGL>
#include <QGLWidget>


class RenderHandler;

#define MAX_BUFF_SIZE	10
typedef struct RENDER_BUFFER {
	int count;
	QMutex *mutex;
	QWaitCondition *cond;
	bool b_waiting;
} RENDER_BUFFER;



class RenderThread : public QThread {
	Q_OBJECT

	public:
		RenderThread(RenderHandler *_handler = 0);
		~RenderThread();

		/* Inherited from QThread */
		void run();

		void stop();

		void init(RenderHandler *_handler);

		bool isRunning();

	public slots:

		/*
		 *	This slot is invoked in the run() method, every time rendering is required
		 */
		void on_render();

	private:
		void acknowledge();

		RenderHandler *handler;
		RENDER_BUFFER buff;
		QMutex mutex;


		bool b_running;

	signals:
		void do_render();
};



class RenderHandler : public QGLWidget {
	Q_OBJECT

	public:
		RenderHandler(QGLFormat format, QWidget *parent) : QGLWidget(format, parent) {}

	public:// slots:
		virtual void render_command() = 0;

};



#endif

