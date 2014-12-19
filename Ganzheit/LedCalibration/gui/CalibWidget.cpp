#include "CalibWidget.h"



void CalibWidget::drawRectAndLED(QImage *_qimg,
								 const calib::LEDCalibContainer &container,
								 const calib::LEDCalibSample &sample,
								 const Camera *cam) {

	const std::vector<cv::Point2f> &image_points = sample.image_points;

	// draw the found markers on the image
	drawMarkers(_qimg, image_points);



    /**************************************************************
     * Get the vectors from the LED calibrator
     **************************************************************/

    std::vector<cv::Point3f> objectPoints;
    calib::LEDCalibPattern::getLEDObjectPoints(container.circleSpacing,
                                               objectPoints);

    Eigen::Matrix4d transformation;

    // compute the transformation matrix
    calib::LEDCalibPattern::makeTransformationMatrix(cam->getIntrisicMatrix(),
                                                     cam->getDistortion(),
                                                     sample.image_points,
                                                     objectPoints,
                                                     transformation);

    // compute the normal
    Eigen::Vector3d eig_normal;
    calib::LEDCalibPattern::computeNormal(transformation, eig_normal);

    // the vector from the camera centre to the intersection point
    const Eigen::Vector3d vec_inters = calib::computeIntersectionPoint(sample.glint,
                                                                       objectPoints,
                                                                       eig_normal,
                                                                       transformation,
                                                                       *cam);

    if(vec_inters(0) == 0 && vec_inters(1) == 0 && vec_inters(2) == 0) {
        return;
    }

    // the vector reflected with respect to the intersection vector.
    Eigen::Vector3d vec_reflect = calib::computeReflectionPoint(eig_normal,
                                                                vec_inters);


    /**************************************************************
     * Get and draw the glint
     **************************************************************/
    const cv::Point2d &glint = sample.glint;

    {

        QPainter painter(_qimg);
        painter.drawArc(glint.x - 3, glint.y - 3, 6, 6, 0, 16*360);


        /**************************************************************
         * Map the reflected vector to 2D and draw it
         **************************************************************/
        double c = 3*container.circleSpacing;

        cv::Point3d p3DStart;
        p3DStart.x = vec_inters(0);
        p3DStart.y = vec_inters(1);
        p3DStart.z = vec_inters(2);

        double uStart, vStart;
        cam->worldToPix(p3DStart, &uStart, &vStart);

        cv::Point3d p3DEnd;
        p3DEnd.x = vec_inters(0) + c*vec_reflect(0);
        p3DEnd.y = vec_inters(1) + c*vec_reflect(1);
        p3DEnd.z = vec_inters(2) + c*vec_reflect(2);

        double uEnd, vEnd;

        cam->worldToPix(p3DEnd, &uEnd, &vEnd);
        painter.setPen(Qt::red);
        painter.drawLine((int)(uStart + 0.5), (int)(vStart + 0.5),
                         (int)(uEnd + 0.5), (int)(vEnd + 0.5));


        /*******************************************************************************
         * Map the normal vector to 2D and draw it
         *******************************************************************************/
        p3DEnd.x = vec_inters(0) + c*eig_normal(0);
        p3DEnd.y = vec_inters(1) + c*eig_normal(1);
        p3DEnd.z = vec_inters(2) + c*eig_normal(2);

        cam->worldToPix(p3DEnd, &uEnd, &vEnd);

        painter.setPen(Qt::blue);
        painter.drawLine((int)(uStart + 0.5), (int)(vStart + 0.5),
                         (int)(uEnd + 0.5), (int)(vEnd + 0.5));

    }


    /**************************************************
     * Compute the transformed object points
     **************************************************/

    // get the normalised object points
    const std::vector<cv::Point3f> &norm_object_points = container.normalisedObjectPoints;

    // define the transformed object points
    std::vector<cv::Point3f> trans_obj_points(norm_object_points.size());

    // compute the transformed object points
    computeTransormedPoints(norm_object_points,
                            container.circleSpacing,
                            transformation,
                            trans_obj_points);

	{

		QPainter painter(_qimg);
		painter.setPen(Qt::yellow);
		int x = 10;
		int y = 15;
		for(size_t i = 0; i < trans_obj_points.size(); ++i) {

			const cv::Point3f &curPoint = trans_obj_points[i];

			QString str;
			str.sprintf("%d:  (%.2f, %.2f, %.2f)", (int)i, curPoint.x, curPoint.y, curPoint.z);
			painter.drawText(x, y, str);
			y += 20;

		}

	}

}


void CalibWidget::computeTransormedPoints(const std::vector<cv::Point3f> &norm_object_points,
										  double circleSpacing,
										  const Eigen::Matrix4d &transformation,
										  std::vector<cv::Point3f> &trans_obj_points) {

	const size_t sz = norm_object_points.size();

	for(size_t i = 0; i < sz; ++i) {

		const cv::Point3f &p = norm_object_points[i];

        Eigen::Vector4d v((double)(circleSpacing * p.x),
						  (double)(circleSpacing * p.y),
                          (double)(circleSpacing * p.z),
						  1.0);



        Eigen::Vector4d res = transformation * v;

		cv::Point3f &curTransPoint = trans_obj_points[i];
		curTransPoint.x = res(0);
		curTransPoint.y = res(1);
		curTransPoint.z = res(2);

	}

}


void CalibWidget::drawMarkers(QImage *_qimg, const std::vector<cv::Point2f> &markers) {

	QPainter painter(_qimg);

	QFont font;
	font.setPixelSize(20);
//			painter.setFont(font);

	int w = 10;
	int w_2 = w / 2;
	for(int i = 0; i < (int)markers.size(); ++i) {

		int x = (int)(markers[i].x + 0.5f - w_2);
		int y = (int)(markers[i].y + 0.5f - w_2);

        painter.setPen(Qt::yellow);
		painter.drawArc(x, y, w, w, 0, 360*16);

		QString s;
		s.sprintf("%d", i);

        painter.setPen(Qt::red);
		painter.drawText(QRect(x-10, y, 200, 20), s);

	}

}

