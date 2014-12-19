#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>


static const double PI = 3.14159265;

inline double DEGTORAD(double deg) {
        return deg * PI / 180.0;
}
void getEllipsePoint(const cv::RotatedRect &ellipse, double ang_p, cv::Point2d &ret);





int main(int argc, char **argv) {

	/**********************************************************************
	 * Initialize the main win
	 *********************************************************************/
	cv::namedWindow ("Main", CV_WINDOW_AUTOSIZE);

	cv::Mat img(480, 640, CV_8UC3);
	double angle = 0.0;


	bool b_running = true;
	while(b_running) {

		int key = cv::waitKey(2);
		if((char)key == 27) {
			b_running = false;
		}
		else if((char)key == 83) {	// right key
			angle += 1.0;
		}
		else if((char)key == 81) {	// left key
			angle -= 1.0;
		}

		cv::RotatedRect r(cv::Point2f(640 / 2., 480 / 2.), cv::Size(90, 160), angle);

		cv::ellipse(img, r, cv::Scalar(0, 0, 255), 2, CV_AA);




		double ang[2];
		if(r.size.width > r.size.height) {	// horisontal axis is the major axis
			ang[0] = 0.0;
			ang[1] = PI;
		}
		else {	// vertical axis is the major axis
			ang[0] = PI / 2.0;
			ang[1] = -PI / 2.0;
		}


		for(double ang = 0; ang <= 3.14; ang += 3.14 / 14) {
			cv::Point2d p;
			getEllipsePoint(r, ang, p);
			cv::circle(img, p, 3, cv::Scalar(255, 100, 155), 1, CV_AA);
		}


		cv::Point2d p1, p2;
		getEllipsePoint(r, ang[0], p1);
		getEllipsePoint(r, ang[1], p2);
		cv::circle(img, p1, 3, cv::Scalar(0, 255, 0), 1, CV_AA);
		cv::circle(img, p2, 3, cv::Scalar(0, 255, 0), 1, CV_AA);




		cv::imshow("Main", img);

		cv::rectangle(img, cv::Point(0, 0), cv::Point(639, 479), cv::Scalar(0, 0, 0), -1);

		char str[256];
		sprintf(str, "%.2f", angle);

		cv::putText(img, std::string(str), cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255));
	}


	return 0;
}



void getEllipsePoint(const cv::RotatedRect &ellipse, double ang_p, cv::Point2d &ret) {

	const double ang_e = DEGTORAD((double)ellipse.angle);

	double R[4] = {cos(ang_e), -sin(ang_e),
				   sin(ang_e), cos(ang_e)};

	double r1 = ellipse.size.width / 2.0;
	double r2 = ellipse.size.height / 2.0;

	double x = r1 * cos(ang_p);
	double y = r2 * sin(ang_p);

	double x_rot = R[0] * x + R[1] * y;
	double y_rot = R[2] * x + R[3] * y;

	ret.x = x_rot + ellipse.center.x;
	ret.y = y_rot + ellipse.center.y;
}


