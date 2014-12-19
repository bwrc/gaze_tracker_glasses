#ifndef CALIB_DATA_WRITER_H
#define CALIB_DATA_WRITER_H


#include "tinyxml.h"
#include <string>
#include "CameraCalibrator.h"
#include "LEDCalibrator.h"


class CalibDataWriter {

public:

    CalibDataWriter();
    ~CalibDataWriter();

    bool create(const std::string &path);
    void close();


    bool writeCameraData(const calib::CameraCalibContainer &cameraData);
    bool writeLEDData(const std::vector<calib::LEDCalibContainer> &LEDData);

private:

    static void vecToStr(const double *vec, int n, std::string &oput);

    std::string saveFile;

    TiXmlDocument doc;
    TiXmlElement *root;

};


#endif

