#include "PupilTracker.h"
#include "localTrackerSettings.h"
#include "trackerSettings.h"
#include <opencv2/highgui/highgui.hpp>
#include "Averager.h"
#include "iris.h"
#include "Timing.h"


static bool openCaputreDevice(cv::VideoCapture &cap, const std::string &strFileVideo);
static void burstIris(cv::Mat &imgGray, const gt::PupilTracker *pTracker);
static void drawEllipse(cv::Mat &imgGray,
                        const cv::RotatedRect &ell,
                        const std::vector<cv::Point2f> &vec,
                        const cv::Scalar & color);

static const double dPI = 3.141592653589;


int main(int nArgs, const char **pArgs) {

    // args
    if(nArgs != 2) {
        printf("Usage:\n    %s <file>\n  Or\n    %s cam=<number of device>\n", pArgs[0], pArgs[0]);
        return -1;
    }

    // video file
    std::string strFileVideo(pArgs[1]);

    // capture device
    cv::VideoCapture cap;

    // open
    if(!openCaputreDevice(cap, strFileVideo)) {
        printf("Could not open the video\n");
        return -1;
    }


    {
        std::string strFileSettings = "default.xml";
        SettingsIO settingsFile(strFileSettings);
        LocalTrackerSettings localSettings;
        localSettings.open(settingsFile);
        trackerSettings.set(localSettings);
    }


    // create the pupil tracker
    gt::PupilTracker *pTracker = new gt::PupilTracker();
    pTracker->useAutoTh(false);
    pTracker->set_nof_crs(6);

    // main loop
    bool bRunning = true;
    while(bRunning) {

        // get frame
        cv::Mat imgBgr;
        cap >> imgBgr;

        // reopen the capture device if the frame is empty
        if(imgBgr.empty()) {

            // release and reopen
            cap.release();
            if(!openCaputreDevice(cap, strFileVideo)) {
                printf("Could not reopen the video\n");
                return -1;
            }

            continue;

        }

        // convert the image to gray
        cv::Mat imgGray;
        cv::cvtColor(imgBgr, imgGray, CV_BGR2GRAY);

        pTracker->track(imgGray);
        burstIris(imgGray, pTracker);

        // display the image in a window
        cv::imshow("iris", imgBgr);

        int key = cv::waitKey(33);
        if((char)key == 'q') {
            bRunning = false;
        }

    }

    delete pTracker;

    return 0;

}


void burstIris(cv::Mat &imgGray, const gt::PupilTracker *pTracker) {

    // const std::vector<cv::Point2d> crs = pTracker->getCornealReflections();
    // for(size_t i = 0; i < crs.size(); ++i) {
    //     cv::circle(imgGray, crs[i], 4, cv::Scalar(0), CV_FILLED, CV_AA);
    // }


    // define the roi
    const cv::Rect roi(trackerSettings.CROP_AREA_X,
                       trackerSettings.CROP_AREA_Y,
                       trackerSettings.CROP_AREA_W,
                       trackerSettings.CROP_AREA_H);

    // crop based on the roi
    cv::Mat imgGrayCrop(imgGray, roi);

    // processing durations
    long lDurBlur  = 0;
    long lDurBurst = 0;

    utils::Timing timing;

    /*
     * Blur the image so that the iris is of constant color.
     * The cv::medianBlur() method is the best, but computationally
     * expensive. Good results can be obtained with cv::blur() too.
     */
    cv::equalizeHist(imgGrayCrop, imgGrayCrop);
    //cv::medianBlur(imgGrayCrop, imgGrayCrop, 27);
    cv::blur(imgGrayCrop, imgGrayCrop, cv::Size(3, 3), cv::Point(-1, -1));
    lDurBlur = timing.getElapsedMicros();


    /**********************************************************************
     * Burst
     **********************************************************************/
    timing.markTime();
    double c = 1.4;
    const cv::RotatedRect *pEll = pTracker->getEllipsePupil();
    cv::Size sz(c * pEll->size.width,
                c * pEll->size.height);

    const cv::RotatedRect ell(pEll->center, sz, pEll->angle);

    const cv::Point2f startPoint = pTracker->getEllipsePupil()->center;
    std::vector<cv::Point2f> vecEdges;
    /*bool bBurstOk =*/ iris::burst(imgGray, roi, ell, vecEdges);
    lDurBurst = timing.getElapsedMicros();


    // print durations
    printf("Blur  : %.2f ms\n", lDurBlur  * 0.001);
    printf("Burst : %.2f ms\n", lDurBurst * 0.001);
    printf("*****************\n");

    /*********************************************************
     * Select the left-right and top-bottom regions of the
     * rays.
     *********************************************************/
    std::vector<cv::Point2f> vecTB; // top-bottom
    for(size_t i = 0; i < vecEdges.size(); ++i) {

        const cv::Point2f &curPoint = vecEdges[i];
        const double x = curPoint.x - startPoint.x;
        const double y = curPoint.y - startPoint.y;

        const double dAng = atan2(y, x);
        const double dAngAbs = fabs(dAng);

        const double dLimit = 0.25*dPI;

        if(dAngAbs > dLimit && (dPI - dAngAbs) > dLimit) { // 0.25 * pi = pi / 4
            vecTB.push_back(curPoint);
        }

    }

    /*************************************************
     * Fit ellipses to both point sets.
     * ellipse fitting requires at least five points.
     *************************************************/
    cv::RotatedRect ellTB = vecTB.size() >= 5 ? cv::fitEllipse(vecTB) : cv::RotatedRect();
    ellTB.size.height *= 0.9;
    ellTB.size.width  *= 0.9;

    if(ellTB.size.width > 0) {
        drawEllipse(imgGray, ellTB, vecTB, cv::Scalar(200));
    }

    cv::imshow("gray", imgGray);

}


void drawEllipse(cv::Mat &imgGray,
                 const cv::RotatedRect &ell,
                 const std::vector<cv::Point2f> &vec,
                 const cv::Scalar &color) {

    // draw the ellipse
    cv::ellipse(imgGray, ell, color, 1, CV_AA);

    // draw the points
    for(size_t i = 0; i < vec.size(); ++i) {

        int x = vec[i].x;
        int y = vec[i].y;

        int ind = y*imgGray.step + x;
        imgGray.data[ind] = 0;

    }

}


bool openCaputreDevice(cv::VideoCapture &cap, const std::string &strFileVideo) {

    if(strstr(strFileVideo.c_str(), "cam=") != NULL) {

        if(strFileVideo.size() > strlen("cam=")) {
            int nId = atoi(&(strFileVideo.c_str()[4]));
            cap.open(nId);
            
        }

    }
    else {
        cap.open(strFileVideo);
    }

    return cap.isOpened();

}

