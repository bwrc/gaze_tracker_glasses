#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include "ellipse.h"


static const double pi = 3.1415926535897932384626433832795;
static float fAngGlobDeg = 0.0;
static const float fAngSpeedDeg = 1.0;


void drawEllipse(cv::Mat &imgBgr, const cv::RotatedRect &ell) {

    cv::ellipse(imgBgr, ell, cv::Scalar(0, 255, 255), 1, CV_AA);

    cv::Point2d pointHor = ellipse::getEllipsePoint(ell, 0.0);
    cv::Point2d pointVer = ellipse::getEllipsePoint(ell, -pi*0.5);

    cv::Point p1((int)ell.center.x, (int)ell.center.y);
    cv::Point p2((int)pointHor.x, (int)pointHor.y);
    cv::Point p3((int)pointVer.x, (int)pointVer.y);

    cv::Scalar colorAxis(100, 100, 255);
    cv::line(imgBgr, p1, p2, colorAxis, 1);
    cv::line(imgBgr, p1, p3, colorAxis, 1);

}


int main() {

    std::cout << "End by pressing 'q'" << std::endl;

    cv::Mat imgBgr = cv::Mat::zeros(640, 480, CV_8UC3);

    std::string strWin = "ellipse";

    cv::namedWindow(strWin, cv::WINDOW_AUTOSIZE);

    cv::Point2f centre(imgBgr.cols * 0.5, imgBgr.rows * 0.5);
    cv::Size sz(imgBgr.cols * 0.2, imgBgr.rows * 0.4);

    float fAngDeg = 30.0;
    static cv::RotatedRect ell(centre, sz, fAngDeg);

    while(1) {

        // clear the screen
        cv::rectangle(imgBgr, cv::Point(0, 0),
                      cv::Point(imgBgr.cols - 1, imgBgr.rows - 1),
                      cv::Scalar(0, 0, 0), CV_FILLED);

        // draw the ellipse
        drawEllipse(imgBgr, ell);

        double dAngCompensateDeg = fAngGlobDeg - fAngDeg;
        double dAngCompensateRad = dAngCompensateDeg*pi / 180.0;
        cv::Point2d p = ellipse::getEllipsePoint(ell, dAngCompensateRad);

        cv::circle(imgBgr, p, 5, cv::Scalar(0, 255, 0), 1, CV_AA);

        {
            std::stringstream ss;
            ss << "ang = " << fAngGlobDeg;
            cv::Point p(10, imgBgr.rows - 10);
            cv::putText(imgBgr, ss.str(), p, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1, CV_AA);

            ss.str(std::string());
            ss << "ang e = " << fAngDeg;
            p = cv::Point(10, imgBgr.rows - 30);
            cv::putText(imgBgr, ss.str(), p, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1, CV_AA);

        }

        cv::Scalar colorAxis(255, 100, 100);
        {
            cv::Point p1(0, imgBgr.rows*0.5);
            cv::Point p2(imgBgr.cols - 1, imgBgr.rows*0.5);
            cv::line(imgBgr, p1, p2, colorAxis, 1);
        }
        {
            cv::Point p1(imgBgr.cols * 0.5, 0);
            cv::Point p2(imgBgr.cols * 0.5, imgBgr.rows-1);
            cv::line(imgBgr, p1, p2, colorAxis, 1);
        }

        cv::imshow(strWin, imgBgr);

        const int nKey = cv::waitKey(16);
        const char cKey = static_cast<char>(nKey);
        if(cKey == 'q') {
            break;
        }
        else if(cKey == 'S') { // right arrow
            fAngGlobDeg += fAngSpeedDeg;
        }
        else if(cKey == 'Q') { // left arrow
            fAngGlobDeg -= fAngSpeedDeg;
        }

    }

    return 0;

}

