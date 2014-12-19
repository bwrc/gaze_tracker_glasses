#ifndef CALIBWIDGET_H
#define CALIBWIDGET_H

#include <QImage>
#include <QPainter>
#include "LEDCalibrator.h"
#include "Camera.h"


class CalibWidget {

	public:

		CalibWidget() {img = NULL;}
		virtual ~CalibWidget() {delete img; img = NULL;}

		virtual QImage *getImage() {return img;}



		virtual void drawRectAndLED(QImage *_qimg,
									const calib::LEDCalibContainer &container,
									const calib::LEDCalibSample &sample,
									const Camera *cam);

		void drawMarkers(QImage *_qimg,
						 const std::vector<cv::Point2f> &markers);


		virtual QSize getSize() = 0;

	protected:

		QImage *img;

	private:

		static void computeTransormedPoints(const std::vector<cv::Point3f> &object_points,
											double circleSpacing,
											const Eigen::Matrix4d &transformation,
											std::vector<cv::Point3f> &trans_obj_points);

};


#endif

