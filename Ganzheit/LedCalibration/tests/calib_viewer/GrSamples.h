#ifndef GR_SAMPLES_H
#define GR_SAMPLE_H

#include <GL/gl.h>
#include "LEDCalibrator.h"


class GrSamples {

public:

    GrSamples();

    void reset(const calib::LEDCalibContainer &container,
               const calib::LEDCalibSample &sample,
               const Camera &cam);

    void computePatternOGLPoints(const Camera &cam,
                                 const std::vector<cv::Point2f> &imagePoints,
                                 double circleSpacing);

    void computeGlintOGLPoint(const Camera &cam,
                              const calib::LEDCalibSample &curSample,
                              double circleSpacing);

    void computeRays(const Camera &cam,
                     const calib::LEDCalibSample &curSample,
                     double circleSpacing);

    void draw();


    static void drawLED(double x, double y, double z);

private:

    void computePatternOGLPoints(const std::vector<cv::Point3f> &objectPoints,
                                 const Eigen::Matrix4d &transformation);


    void computeGlintOGLPoint(const Camera &cam,
                              const Eigen::Matrix4d &tm,
                              const std::vector<cv::Point3f> &objectPoints,
                              const calib::LEDCalibSample &sample);

    void computeRays(const Camera &cam,
                     const Eigen::Matrix4d &tm,
                     std::vector<cv::Point3f> &objectPoints,
                     const calib::LEDCalibSample &sample);


    void drawCamera();
    void drawLED();

    double glintOGLPoint[3];
    double patternOGLPoints[3*16];
    double vecReflection[3];
    double vecIntersection[3];
    double posLED[3];

};


#endif

