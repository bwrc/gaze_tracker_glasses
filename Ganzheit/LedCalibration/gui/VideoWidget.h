#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H


#include "VideoStreamer.h"
#include "streamer_structs.h"
#include "mainwindow.h"
#include "CalibWidget.h"
#include <QPaintEvent>
#include <string>


class MainWindow;


class VideoWidget : public StreamHandler, public CalibWidget {
	Q_OBJECT

	public:

		VideoWidget(MainWindow *parent);
		~VideoWidget();

		VideoStreamer *getStreamer() {return streamer;}

		/*
		 *	Inherited from StreamHandler
		 */
		void onFrameChanged(STREAM_FRAME_CHANGED_DATA *data);


		/*
		 *	Inherited from StreamHandler
		 */
		void onStreamInit(STREAM_INIT_DATA *init_data);

		void begin();//const std::string &_source);


		void changeSource(const std::string &_source, int w, int h);

		QSize getSize() {return size();}

	private:
		void paintEvent(QPaintEvent *evt);

		MainWindow *mainWin;
		VideoStreamer *streamer;

		std::string source;
};


#endif

