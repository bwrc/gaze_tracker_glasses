#ifndef CalibDataReader_H
#define CalibDataReader_H


#include <tinyxml.h>
#include <string>
#include "CameraCalibSample.h"
#include "LEDCalibSample.h"
#include <list>
#include <vector>


class CalibDataReader {

public:

    CalibDataReader();
    ~CalibDataReader();

    bool create(const std::string &path);

    bool readCameraContainer(calib::CameraCalibContainer &container);
    bool readLEDContainers(std::vector<calib::LEDCalibContainer> &containers);

private:

    void sortLEDContainersByID(std::vector<calib::LEDCalibContainer> &containers);

    bool readLEDContainer(TiXmlElement *elLED, calib::LEDCalibContainer &container);

    /*
     * Used by readCameraContainer when the first <sample> element has been
     * found in the <camera> element
     */
    bool readCameraSamples(TiXmlElement *elCamera, calib::CameraCalibContainer &container);

    /*
     * Used by readLEDContainer when the first <sample> element has been
     * found in the <LED> element
     */
    bool readLEDSamples(TiXmlElement *elSample, calib::LEDCalibContainer &container);

    static bool strToVec(const std::string &str, double *vec, int n);
    static bool strToVec(const std::string &str, float * vec, int n);
    static bool _2dPointStrToVec(const std::string &str_image_points, std::vector<cv::Point2f> &image_points);
    static bool _3dPointStrToVec(const std::string &str_object_points, std::vector<cv::Point3f> &object_points);
    static double toDouble(const char *str, bool *ok);
    static void split(const std::string &str, std::vector<std::string> &strList, const char delim);
    static void split(const std::string &str, std::list<std::string> &strList, const char delim);
    static void remove(std::string &str, char c);

    TiXmlDocument doc;

};


#endif

