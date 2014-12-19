#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "ellipse.h"


static const double pi = 3.1415926535897932384626433832795;
static const cv::Scalar colorIn(255, 0, 255);
static const cv::Scalar colorOut(255, 255, 0);
static cv::Scalar color = colorOut;
static double dAng = 0.0;
static cv::Point mousePoint;


static void onMouse(int event, int x, int y, int, void*) {

    mousePoint.x = x;
    mousePoint.y = y;

}


int main() {

    std::cout << "End by pressing 'q'" << std::endl;

    cv::Mat imgBgr = cv::Mat::zeros(640, 480, CV_8UC3);

    std::string strWin = "ellipse";

    cv::namedWindow(strWin, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(strWin, onMouse, 0);

    cv::Point2f centre(imgBgr.cols * 0.5, imgBgr.rows * 0.5);
    cv::Size sz(imgBgr.cols * 0.6, imgBgr.rows * 0.2);
    static cv::RotatedRect ell(centre, sz, dAng);


    while(1) {

        cv::rectangle(imgBgr, cv::Point(0, 0),
                      cv::Point(imgBgr.cols - 1, imgBgr.rows - 1),
                      cv::Scalar(0, 0, 0), CV_FILLED);

        cv::Point2f p(mousePoint.x, mousePoint.y);
        if(ellipse::pointInsideEllipse(ell, p)) {
            color = colorIn;
        }
        else {
            color = colorOut;
        }


        cv::ellipse(imgBgr, ell, color, 1, CV_AA);


        cv::imshow(strWin, imgBgr);

        int nKey = cv::waitKey(16);
        if(static_cast<char>(nKey) == 'q') {

            break;

        } 

        dAng += 1;
        ell.angle = dAng;

    }

    return 0;

}

