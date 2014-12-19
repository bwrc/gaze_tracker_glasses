#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QWidget>
#include <QRadioButton>
#include <QMenuBar>
#include <QActionGroup>
#include <QFileDialog>
#include <QEvent>
#include <QChildEvent>
#include <QKeyEvent>
#include <QStatusBar>
#include <QMainWindow>

#include "VideoWidget.h"
#include "ControlPanel.h"
#include "CameraCalibrator.h"
#include "LEDCalibrator.h"
#include "Camera.h"
#include "ResultItem.h"
#include "ThumbNailPanel.h"



class GLWidget;
class VideoWidget;
class ResultItem;



class MainWindow : public QMainWindow {

    Q_OBJECT

	public:

    MainWindow();

    /*
     * Called by the VideoWidget, when it receives a frame.
     * The image can be modified.
     */
    void cbFrameReceived(QImage *imgRgb);

public slots:

    /* Menu slots */
    void onMenuFileOpen();
    void onMenuFileSave();
    void onMenuFileNew();

    bool locateCircles(QImage *img_glw);
    void doCalibrate();

    bool locateRectAndLED(QImage *qimgRGB);
    void addCalibData();

    void resetCalibration();

    void video_source_changed();

    void onCalibItemChanged(int item);

private:

    void populateThumbNailPanel(const std::vector<calib::LEDCalibSample> &samples);
    void populateThumbNailPanel(const std::vector<calib::CameraCalibSample> &samples);

    void resizeEvent(QResizeEvent *evt);
    bool eventFilter(QObject *object, QEvent *event);

    void createMenu();
    void makeConnections();
    void adjustLayout(void);

    VideoWidget *video_widget;

    ControlPanel *controlPanel;

    cv::Mat img_gray;

    QStatusBar * statusbar;

    // menu-related components
    QMenuBar *menubar;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionNew;

    ThumbNailPanel *tn_panel;
    QGraphicsView *tn_view;

    /*
     * This is a copy of the image received from the video streamer.
     * This image is not to be modified, since it will be saved, and
     * possibly reanalysed later.
     */
    QImage imgVideoFrame;

};


#endif // MAINWINDOW_H

