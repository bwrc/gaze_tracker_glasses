#include "VideoWidget.h"


VideoWidget::VideoWidget(MainWindow *parent) : StreamHandler((QWidget *)parent), CalibWidget() {

	mainWin		= parent;
	img			= NULL;
	streamer	= new VideoStreamer(this);


	setPalette(QPalette(Qt::blue));
}


// no need to delete image since it will be deleted in CalibWidget
VideoWidget::~VideoWidget() {
	streamer->stopCapture();
	streamer->wait();
	delete streamer;
}


void VideoWidget::begin() {//const std::string &_source, int w, int h) {

//	source = _source;

//	streamer->setVideoDim(w, h);
//	streamer->getSettings()->setInput(source.c_str());
//	streamer->startCapture();

	// start the thread
	streamer->start();

}


void VideoWidget::changeSource(const std::string &_source, int w, int h) {

	source = _source;

	streamer->pauseCapture();
	streamer->setVideoDim(w, h);
	streamer->getSettings()->setInput(source.c_str());
	streamer->startCapture();

}


/*
 *	Inherited from StreamHandler
 */
void VideoWidget::onFrameChanged(STREAM_FRAME_CHANGED_DATA *data) {

	QImage img_temp((unsigned char *)data->img_rgb->data,
					data->img_rgb->cols,	// width
					data->img_rgb->rows,	// height
					3*data->img_rgb->cols,	// bytes per line
					QImage::Format_RGB888);	// format

	{
		/*	
		 *	Must be within this scope, so that the QPainter will be
		 *	released before a call to repaint(), because it calls paintEvent(),
		 *	which instantiates a new QPainter. There can be only one instance
		 *	of a QPainter at any given time.
		 */
		QPainter painter(img);
		painter.drawImage(0, 0, img_temp);
	}


    // let the mainWin do something, like draw on the image first
    mainWin->cbFrameReceived(img);

	// paint immediately
	this->repaint();

}


/*
 *	Inherited from StreamHandler
 */
void VideoWidget::onStreamInit(STREAM_INIT_DATA *init_data) {
	if(img != NULL) {
		delete img;
		img = NULL;
	}

	img = new QImage(init_data->w, init_data->h, QImage::Format_RGB888);
}


void VideoWidget::paintEvent(QPaintEvent *) {

	if(img != NULL) {

		QPainter painter(this);

		// scale if the image is different in size than the current area
		int img_w = img->size().width();
		int img_h = img->size().height();

		int w = size().width();
		int h = size().height();

		double ratio_w = (double)w / (double)img_w;
		double ratio_h = (double)h / (double)img_h;

		double min_ratio = std::min(ratio_w, ratio_h);

		painter.scale(min_ratio, min_ratio);

		int offx = (size().width()  - img->size().width()  * min_ratio) / 2.0;
		int offy = (size().height() - img->size().height() * min_ratio) / 1.0;

		// then render it
		painter.drawImage(offx, offy, *img);

	}

}

