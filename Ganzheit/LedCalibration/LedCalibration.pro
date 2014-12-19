#-------------------------------------------------
#
# Project created by QtCreator 2011-02-03T11:53:14
#
#-------------------------------------------------

QT       += core gui xml

#OPENCV_DIR=/usr/local/src/OpenCV-2.4.0/build/
OPENCV_DIR=../../opencv/build/


CONFIG( debug, debug|release ) {
	OPENCV_DIR = $${OPENCV_DIR}debug/
	message(Building a debug application)
}
else {
	OPENCV_DIR = $${OPENCV_DIR}release/
	message(Building a release application)
}

LIBS += -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lopencv_features2d -lm -L$${OPENCV_DIR}lib/ `gsl-config --libs`


INCLUDEPATH += $${OPENCV_DIR}include

TARGET = LedCalibration
TEMPLATE = app


INCLUDEPATH += ./../../Eigen3/
INCLUDEPATH += ./../../QVideo/
INCLUDEPATH += ./../TwoCameraTracker/io/
INCLUDEPATH += ./../../tinyxml/
INCLUDEPATH += gui/




SOURCES +=	gui/main.cpp									\
			gui/mainwindow.cpp								\
            ./../TwoCameraTracker/io/CalibDataReader.cpp    \
            ./../TwoCameraTracker/io/CalibDataWriter.cpp    \
			gui/ResultItem.cpp								\
			Camera.cpp										\
			LEDCalibrator.cpp								\
			gui/CalibWidget.cpp								\
			CameraCalibrator.cpp							\
			gui/ControlPanel.cpp							\
			RectangleEstimator.cpp							\
			gui/VideoWidget.cpp								\
			gui/ThumbNailPanel.cpp							\
			./../../QVideo/VideoStreamer.cpp				\
			./../../QVideo/FrameBuffer.cpp					\
			LEDCalibPattern.cpp                             \
			imgproc.cpp	                                    \
			LEDTracker.cpp									\
            ../../tinyxml/tinyxml.cpp                       \
            ../../tinyxml/tinystr.cpp                       \
            ../../tinyxml/tinyxmlerror.cpp                  \
            ../../tinyxml/tinyxmlparser.cpp



HEADERS +=	gui/mainwindow.h								\
			gui/ControlPanel.h								\
            ./../TwoCameraTracker/io/CalibDataReader.h      \
            ./../TwoCameraTracker/io/CalibDataWriter.h      \
			gui/ResultItem.h								\
			CameraCalibrator.h								\
			Camera.h										\
			LEDCalibrator.h									\
			gui/CalibWidget.h								\
			RectangleEstimator.h							\
			gui/ThumbNailPanel.h							\
			./../../QVideo/VideoStreamer.h					\
			./../../QVideo/FrameBuffer.h					\
			./../../QVideo/streamer_structs.h				\
			LEDCalibPattern.h					\
			imgproc.h							\
			gui/VideoWidget.h								\
			TargetTracker.h									\
			LEDTracker.h									\
            ../../tinyxml/tinyxml.h                       \
            ../../tinyxml/tinystr.h

QT += widget
