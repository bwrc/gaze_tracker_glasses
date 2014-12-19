#include "mainwindow.h"
#include "CalibWidget.h"

#include <QInputDialog>
#include <QMessageBox>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>



/**********************************************
 * GUI dimensions
 **********************************************/
static const int WIDTH_VIDEO	= 640;
static const int HEIGHT_VIDEO	= 480;
static const int HEIGHT_MENU	= 30;


/**********************************************
 * Other variables
 **********************************************/
static calib::LEDCalibSample	curLEDSample;
static calib::CameraCalibSample	curCamSample;



MainWindow::MainWindow() : QMainWindow() {

	/*************************************************
	 * Create the video widget
	 *************************************************/
	video_widget = new VideoWidget(this);
	video_widget->setGeometry(5, HEIGHT_MENU + 5, WIDTH_VIDEO, HEIGHT_VIDEO);
	video_widget->show();
	video_widget->begin();


	/*************************************************
	 * Get the status bar
	 *************************************************/
	statusbar = this->statusBar();


	/*************************************************
	 * Create the control panel
	 *************************************************/
	controlPanel = new ControlPanel(this);
	controlPanel->videoSource->setEnabled(true);


	// set the sliders
	controlPanel->slider_th_reflection->setValue(200);
	controlPanel->slider_th_rect->setValue(30);


	/**************************************************************
	 * Create the menu
	 **************************************************************/
	createMenu();


	/**************************************************************
	 * Create the thumbnail panel
	 **************************************************************/
	tn_panel = new ThumbNailPanel();
	tn_panel->installEventFilter(this);
	tn_view = new QGraphicsView(tn_panel, this);
	tn_view->setRenderHint(QPainter::Antialiasing);
	tn_view->setCacheMode(QGraphicsView::CacheBackground);
	tn_view->show();


	/**************************************************************
	 * Background colour
	 **************************************************************/
	QPalette pal(palette());
	pal.setColor(QPalette::Window, Qt::black);
	pal.setColor(QPalette::Button, Qt::white);
	pal.setColor(QPalette::WindowText, Qt::red);
	setAutoFillBackground(true);
	setPalette(pal);


	/*************************************************
	 * Make connections
	 *************************************************/
	makeConnections();


	installEventFilter(this);

}


void MainWindow::createMenu() {

	// create the menubar and add items to it
	this->menubar = new QMenuBar(this);
	QMenu *fileMenu = new QMenu(tr("&File"), this);

	actionOpen	= fileMenu->addAction(tr("&Open"));
	actionSave	= fileMenu->addAction(tr("&Save"));
	actionNew	= fileMenu->addAction(tr("&New"));

	this->menubar->addMenu(fileMenu);

}


void MainWindow::makeConnections() {

	connect(actionOpen, SIGNAL(triggered()), this, SLOT(onMenuFileOpen()));
	connect(actionSave, SIGNAL(triggered()), this, SLOT(onMenuFileSave()));
	connect(actionNew, SIGNAL(triggered()), this, SLOT(onMenuFileNew()));


    connect(controlPanel->btn_add_calib_img, SIGNAL(clicked()),
            this, SLOT(addCalibData()));

    connect(controlPanel->btn_calibrate, SIGNAL(clicked()),
            this, SLOT(doCalibrate()));

	connect(controlPanel->videoSource, SIGNAL(currentIndexChanged(int)),
			this, SLOT(video_source_changed()));

    connect(controlPanel->btn_reset_calibration, SIGNAL(clicked()),
            this, SLOT(resetCalibration()));

	connect(controlPanel->list_calib, SIGNAL(itemSelectionChanged()),
            controlPanel, SLOT(listItemChanged()));

}


void MainWindow::video_source_changed() {

	QString qsource = controlPanel->videoSource->currentText();

	std::string source(qsource.toUtf8().data());

	video_widget->changeSource(source, WIDTH_VIDEO, HEIGHT_VIDEO);

}


bool MainWindow::locateRectAndLED(QImage *_qimgRGB) {

	ResultItem *curItem = controlPanel->getResultItem();

	if(curItem == NULL) {
        statusbar->showMessage(QString("No item"));
		return false;
	}

	// create an OpenCV header for the image
	cv::Mat ocvImgRGB = cv::Mat(_qimgRGB->height(),			// rows
                                _qimgRGB->width(),			// cols
                                CV_8UC3,						// 8-bit uchar, 3 channels
                                (uchar*)_qimgRGB->bits(),	// data
                                _qimgRGB->bytesPerLine());	// bytes_per_row = bytes() / height()

	Camera cam = curItem->getCamCalibContainer().makeCameraObject();


	// extract the features from the image
	bool bTrackOk = calib::extractFeatures(
                                   ocvImgRGB,
                                   curLEDSample,
                                   (unsigned char)controlPanel->slider_th_rect->value(),
                                   (unsigned char)controlPanel->slider_th_reflection->value());


	// make the image into gray if requested
	if(controlPanel->checkBoxGray->isChecked()) {

		unsigned char th = controlPanel->slider_th_rect->value();
		cv::Mat gray;
		cv::cvtColor(ocvImgRGB, gray, CV_RGB2GRAY);
		cv::threshold(gray, gray, th, 255, cv::THRESH_BINARY);
		cv::cvtColor(gray, ocvImgRGB, CV_GRAY2RGB, 3);

	}

	if(bTrackOk) {

        statusbar->showMessage(QString("Track ok"));

	}
    else {

    }

    return bTrackOk;

}


bool MainWindow::locateCircles(QImage *img_calib) {

	if(img_calib == NULL) {
		return false;
	}

	// create an OpenCV header, does not copy data
	cv::Mat cv_rgb(img_calib->height(), img_calib->width(), CV_8UC3, img_calib->bits());

    cv::Mat imgGray;

	// convert to grayscale
	cv::cvtColor(cv_rgb, imgGray, CV_RGB2GRAY);

	// locate the circle pattern
	if(!calib::CameraCalibrator::findCircles(imgGray, curCamSample)) {

		// show the error on the status bar
		QString s;

		return false;

	}

    statusbar->showMessage(QString("Track ok"));

    return true;

}


void MainWindow::resetCalibration() {

	ResultItem *curItem = controlPanel->getResultItem();

	if(curItem == NULL) {
		return;
	}


	ResultItem::CalibrationMode calibMode = curItem->getCalibrationMode();

	if(calibMode == ResultItem::ModeCamera) {

		curItem->clearCamCalibContainer();

		statusbar->showMessage("Samples for camera calibration removed.");

	}
	else if(calibMode == ResultItem::ModeLED) {

		curItem->clearLEDCalibContainer();

		statusbar->showMessage("Samples for LED calibration removed");

	}
	else {
		return;
	}

	// clear the thumbnails
	tn_panel->clear();

}


void MainWindow::addCalibData() {

	ResultItem *curItem = controlPanel->getResultItem();

	int state = controlPanel->checkBoxCalibrate->checkState();

	if(curItem == NULL ||
	   state == Qt::Unchecked) {

		return;

	}

	ResultItem::CalibrationMode calibMode = curItem->getCalibrationMode();


	if(calibMode == ResultItem::ModeCamera) {

		if(curCamSample.image_points.size() == 44) {

			ThumbNail*tn = new ThumbNail(&imgVideoFrame);
			tn_panel->addItem(tn);

			// add the data
			curItem->addCamCalibData(curCamSample, &imgVideoFrame);

			// how many samples are there
			int nof_samples = (int)curItem->getCamCalibContainer().getSamples().size();

			// update the status bar
			QString str;
			str.sprintf("Got %d image(s)", nof_samples);
			statusbar->showMessage(str);

		}
		else {


		}

	}
	else {

		// all image points must have been found
		if(curLEDSample.image_points.size() == 16) {

			ThumbNail *tn = new ThumbNail(&imgVideoFrame);
			tn_panel->addItem(tn);

			// add the sample to the item
			curItem->addLEDCalibData(curLEDSample, &imgVideoFrame);

			// how many samples are there
			int nof_samples = (int)curItem->getLEDCalibContainer().getSamples().size();

			// update the status bar
			QString str;
			str.sprintf("Got %d image(s)", nof_samples);
			statusbar->showMessage(str);

		}
		else {

			// show the error on the status bar
			QString s;

            //			s.sprintf("%s", led_calib.getErrorMsg().c_str());
			statusbar->showMessage(s);

		}

	}

}


void MainWindow::doCalibrate() {

	ResultItem *curItem = controlPanel->getResultItem();
	int state = controlPanel->checkBoxCalibrate->checkState();


	if(curItem == NULL ||
	   state == Qt::Unchecked) {

		return;

	}

	ResultItem::CalibrationMode calibMode = curItem->getCalibrationMode();

	if(calibMode == ResultItem::ModeCamera) {

		// perform calibration
		if(!curItem->calibrateCamera()) {


		}
		else {

		}

	}
	else {

		// compute the LED position using the samples
		curItem->calibrateLED();

	}

}


void MainWindow::onCalibItemChanged(int item) {

	ResultItem *curItem = controlPanel->getResultItem();
	if(curItem == NULL) {
		return;
	}

	if(item == ResultItem::ItemLED) {

		const std::vector<calib::LEDCalibSample> &samples =
			curItem->getLEDCalibContainer().getSamples();

		populateThumbNailPanel(samples);

	}
	else if(item == ResultItem::ItemCamera) {

		const std::vector<calib::CameraCalibSample> &samples =
			curItem->getCamCalibContainer().getSamples();

		populateThumbNailPanel(samples);

	}

}


void MainWindow::populateThumbNailPanel(const std::vector<calib::LEDCalibSample> &samples) {

	// clear the thumbnail panel
	tn_panel->clear();

	size_t sz = samples.size();
	for(size_t i = 0; i < sz; ++i) {

		ThumbNail*tn = new ThumbNail(samples[i].img_path.c_str());
		tn_panel->addItem(tn);

	}

}


void MainWindow::populateThumbNailPanel(const std::vector<calib::CameraCalibSample> &samples) {

	// clear the thumbnail panel
	tn_panel->clear();

	size_t sz = samples.size();
	for(size_t i = 0; i < sz; ++i) {

		const std::string &str = samples[i].img_path;
		const char *cstr = str.c_str();

		QString qstr = QString::fromAscii(cstr);

		ThumbNail*tn = new ThumbNail(qstr);
		tn_panel->addItem(tn);

	}

}


void MainWindow::resizeEvent(QResizeEvent *) {

	// adjust the layout
	adjustLayout();

}


void MainWindow::adjustLayout() {

	static const int controlPanel_W	= 400;
	static const int controlPanel_H	= 600;
	static const int BORDER_W			= 5;

	// Calculate the width of the window
	int mySize_w = WIDTH_VIDEO + controlPanel_W + 3*BORDER_W;


	/********************************************************************** 
	 * Menubar, upper part of the window
	 **********************************************************************/
	this->menubar->setGeometry(0, 0, size().width(), HEIGHT_MENU);


	/********************************************************************** 
	 * The video frame, below the menu bar, on the left side of the window
	 *********************************************************************/
	QRect r;

	r.setRect(BORDER_W, HEIGHT_MENU + BORDER_W, WIDTH_VIDEO, HEIGHT_VIDEO);
	video_widget->setGeometry(r);



	/********************************************************************** 
	 * control-panel is placed on the right part of the window
	 *********************************************************************/

	int controlPanel_h = controlPanel_H;
	int controlPanel_x = r.x() + r.width() + BORDER_W;
	int controlPanel_y = r.y();
	int controlPanel_w = controlPanel_W;

	controlPanel->setGeometry(controlPanel_x, controlPanel_y,
							   controlPanel_w, controlPanel_h);


	/*********************************************************************
	 * And below that is the thumbnail panel
	 *********************************************************************/
	int tn_x = BORDER_W;
	int tn_y = controlPanel_y + controlPanel_h + BORDER_W;
	int tn_h = 100;
	int tn_w = mySize_w - 2*BORDER_W;

	tn_view->setGeometry(tn_x, tn_y, tn_w, tn_h);
	tn_view->setAlignment(Qt::AlignLeft);


	/********************************************************************** 
	 * Update the size of this window
	 *********************************************************************/
	// (Width was calculated before... calculate just the heigth)
	int mySize_h = HEIGHT_MENU + tn_y + tn_h;

	setFixedSize(QSize(mySize_w, mySize_h));

}


void MainWindow::cbFrameReceived(QImage *imgRgb) {

	int state = controlPanel->checkBoxCalibrate->checkState();

	if(state == Qt::Unchecked) {
        return;
    }

	ResultItem *curItem = controlPanel->getResultItem();

	if(curItem == NULL) {

		return;

	}

    // copy the image
    imgVideoFrame = imgRgb->copy();


	ResultItem::CalibrationMode calibMode = curItem->getCalibrationMode();

	if(calibMode == ResultItem::ModeCamera) {

		if(locateCircles(imgRgb)) {

            // get the circles
            cv::Size board_sz = calib::CameraCalibrator::getBoardSize();

            // a cv::Mat header
            cv::Mat ocvRgb(imgRgb->height(),   // rows
                           imgRgb->width(),    // cols
                           CV_8UC3,            // type
                           imgRgb->bits(),     // data
                           3*imgRgb->width()); // step


            // draw the circleboard corners to the cv::Mat
            cv::drawChessboardCorners(ocvRgb, board_sz, curCamSample.image_points, true);

        }

	}
	else if(calibMode == ResultItem::ModeLED) {

		if(locateRectAndLED(imgRgb)) {

            const ResultItem *curItem = controlPanel->getResultItem();

            const Camera cam = curItem->getCamCalibContainer().makeCameraObject();

            // extract the features from the image
            const calib::LEDCalibContainer &container = curItem->getLEDCalibContainer();

            video_widget->drawRectAndLED(imgRgb,
                                         container,
                                         curLEDSample,
                                         &cam);

        }

	}

}


void MainWindow::onMenuFileOpen() {

	// if video on, pause it, so that it will not try to send data to the GUI
	video_widget->getStreamer()->pauseCapture();

	// select the configuration / parameter directory
	QString calibFile = QFileDialog::getOpenFileName(
										this,
										tr("Choose a directory"),
										"",
										tr("calibration (*.calib)"));

	// see if anything was chosen
	if(calibFile.isNull()) {
		return;
	}

	QDir calibDir = QFileInfo(calibFile).dir();

	// now verify that this file has not been selected already
	if(controlPanel->contains(calibDir.absolutePath())) {

		QMessageBox msgBox;
		msgBox.setText("Project has already been chosen");
		msgBox.exec();

		return;
	}


	/* load the configuration contents */
	ResultItem *item = new ResultItem();

	if(!item->init(calibDir.absolutePath(),
				   std::vector<cv::Point3f>(),
				   std::vector<cv::Point3f>(),
				   cv::Size(WIDTH_VIDEO, HEIGHT_VIDEO))) {

		delete item;

		QMessageBox msgBox;
		msgBox.setText("Error creating directory tree, protected directory?");
		msgBox.exec();

		return;

	}

	// load contents
	if(!item->load()) {

		delete item;

		QMessageBox msgBox;
		msgBox.setText("Error loading contents");
		msgBox.exec();

		return;

	}

	// insert the item
	controlPanel->insertNewItem(item);


	// set to idle and enable some
	controlPanel->list_calib->setEnabled(true);


	controlPanel->btn_calibrate->setEnabled(false);
	controlPanel->btn_add_calib_img->setEnabled(false);
	controlPanel->btn_reset_calibration->setEnabled(false);

	// if video was on, resume it
	video_widget->getStreamer()->startCapture();

}


void MainWindow::onMenuFileSave() {

	ResultItem *curItem = controlPanel->getResultItem();

	if(curItem != NULL) {
		curItem->save();
	}

}


void MainWindow::onMenuFileNew() {

	// select a new directory where to place the data
	QString calibDir = QFileDialog::getExistingDirectory(this,
														 tr("Create"),
														 "",
														 QFileDialog::ShowDirsOnly);

	if(calibDir.isNull()) {
		return;
	}

	QDir d = QDir(calibDir);
	QStringList list = d.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

	/* make sure that the directory is empty, otherwise do nothing */
	if(list.count() != 0) {

		QMessageBox msgBox;
		msgBox.setText("Directory is not empty");
		msgBox.exec();

		return;
	}

	// create a new calibration item
	std::vector<cv::Point3f> camera_object_points;
	calib::CameraCalibrator::getObjectPoints(camera_object_points);

	std::vector<cv::Point3f> LED_object_points;
    calib::LEDCalibPattern::getNormalisedLEDObjectPoints(LED_object_points);

	ResultItem *item = new ResultItem();

	if(!item->init(calibDir,
				   camera_object_points,
				   LED_object_points,
				   cv::Size(WIDTH_VIDEO, HEIGHT_VIDEO))) {

		delete item;

		QMessageBox msgBox;
		msgBox.setText("Error creating directory tree, protected directory?");
		msgBox.exec();

		return;

	}


	// insert the item
	controlPanel->insertNewItem(item);

	// enable all widgets
	controlPanel->enableAll();

	// but disable some
	controlPanel->btn_calibrate->setEnabled(false);
	controlPanel->btn_add_calib_img->setEnabled(false);
	controlPanel->btn_reset_calibration->setEnabled(false);

}


bool MainWindow::eventFilter(QObject *target, QEvent *event) {

	/***********************************************************
	 * The thumbnail panel had the focus
	 ***********************************************************/
	if(target == tn_panel) {

		/***********************************************************
		 * A key was pressed
		 ***********************************************************/
		if(event->type() == QEvent::KeyPress) {

			// get the key event
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

			/***********************************************************
			 * Delete key was pressed
			 ***********************************************************/
			if(keyEvent->key() == Qt::Key_Delete) {

				// remove the selected items and get their indices
				std::list<int> indices;
				tn_panel->removeSelected(indices);

				// get the current item
				ResultItem *curItem = controlPanel->getResultItem();

				// the current mode
				ResultItem::CalibrationMode curMode = curItem->getCalibrationMode();

				// next remove the correspondig samples
				if(curMode == ResultItem::ModeCamera) {

					curItem->deleteCameraSamples(indices);

				}
				else if(curMode == ResultItem::ModeLED) {

					curItem->deleteLEDSamples(indices);

				}

			}

//			return true; // filter the event out
			return false; // let the event go further

		}

		/***********************************************************
		 * Another object had the focus
		 ***********************************************************/
		else {

			return false; // handle event further

		}

	}
	else {

		// pass the event on to the parent class
		return QMainWindow::eventFilter(target, event);

	}

}

