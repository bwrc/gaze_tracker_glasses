#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "LEDCalibrator.h"
#include "LEDCalibPattern.h"
#include "CalibDataReader.h"
#include <stdio.h>
#include "CalibDataWriter.h"
#include "InputParser.h"


bool readCalibData(const std::string &calibFile,
                   calib::CameraCalibContainer &camContainer,
                   std::vector<calib::LEDCalibContainer> &LEDContainers);


void enumerateLEDImagePoints(std::vector<calib::LEDCalibContainer> &LEDContainers);

void printCameraData(const calib::CameraCalibContainer &container);
void printLEDData(const std::vector<calib::LEDCalibContainer> &containers);

void printUsageInfo();

bool handleInputParameters(int  argc, const char **args);
bool handleParameter(const ParamAndValue &pair);

bool fileExists(const char *filename);


std::string inputFile;
std::string outputFile;
bool bPrintHelp = false;




int main(int argc, const char **argv) {

    /*
     * At least 1 parameter required
     */
    if(argc < 2) {

        printUsageInfo();

        return -1;

    }


    /*
     * Handle cmd arguments
     */
    if(!handleInputParameters(argc, argv)) {

        printUsageInfo();

        return -1;

    }


    /*
     * Input and output files must be defined
     */

    if(inputFile.empty() || outputFile.empty()) {

        printf("Must specify both, input and output files\n");
        printUsageInfo();

        return -1;
    }

    /*
     * Input and output files must differ
     */
    if(inputFile == outputFile) {
        printf("Input file must not be the same as the output file\n");
        return -1;
    }


    if(fileExists(outputFile.c_str())) {

        std::string input;
        printf("************************************************************\n");
        printf("File %s exists, overwrite (yes/no)\n", outputFile.c_str());
        printf("************************************************************\n");

        while(input != "yes" && input != "no") {

            std::getline(std::cin, input);

            if(input == "no") {

                printf("Run the program and change the -o option\n");

                return -1;

            }
            else if(input == "yes") {

                printf("overwriting %s\n", outputFile.c_str());

                break;

            }

        }

    }


    calib::CameraCalibContainer camContainer;
    std::vector<calib::LEDCalibContainer> LEDContainers;
    if(!readCalibData(inputFile.c_str(), camContainer, LEDContainers)) {
        printf("Could not read %s\n", inputFile.c_str());
        return -1;
    }

    int imgW = camContainer.imgSize.width;

    calib::CameraCalibContainer camContainerFlip = camContainer; // copy

    {

        /******************************************************************
         * Flip the camera samples
         ******************************************************************/
        std::vector<calib::CameraCalibSample> &camSamplesFlip = camContainerFlip.getSamples();

        for(int i = 0; i < (int)camSamplesFlip.size(); ++i) {

            std::vector<cv::Point2f> &imagePointsFlip = camSamplesFlip[i].image_points;

            // invert
            for(size_t j = 0; j < imagePointsFlip.size(); ++j) {
                imagePointsFlip[j].x = imgW - imagePointsFlip[j].x - 1;
            }


            for(int row = 0; row < 5; ++row) {

                for(int col = 0; col < 4; ++col) {

                    int nInd1 = row * 4 + col;
                    int nInd2 = (11 - row - 1) * 4 + col;

                    cv::Point2f &refP1 = imagePointsFlip[nInd1];
                    cv::Point2f &refP2 = imagePointsFlip[nInd2];

                    cv::Point2f tmp = refP1;
                    refP1 = refP2;
                    refP2 = tmp;

                }

            }


        }

    }


    /******************************************************************
     * Flip the LED samples
     ******************************************************************/

    // copy
    std::vector<calib::LEDCalibContainer> LEDContainersFlip = LEDContainers;

    for(size_t cContainer = 0; cContainer < LEDContainersFlip.size(); ++cContainer) {

        std::vector<calib::LEDCalibSample> &LEDSamplesFlip = LEDContainersFlip[cContainer].getSamples();

        for(int cSample = 0; cSample < (int)LEDSamplesFlip.size(); ++cSample) {

            calib::LEDCalibSample &curSample = LEDSamplesFlip[cSample];

            std::vector<cv::Point2f> &imagePointsFlip = curSample.image_points;
            cv::Point2d &glintFlip = curSample.glint;

            glintFlip.x = imgW - glintFlip.x - 1;

            // invert
            for(size_t j = 0; j < imagePointsFlip.size(); ++j) {
                imagePointsFlip[j].x = imgW - imagePointsFlip[j].x - 1;
            }

        }

    }

    // enumerate the image points
    enumerateLEDImagePoints(LEDContainersFlip);


    {

        /******************************************************************
         * Calibrate the camera with the fipped samples
         ******************************************************************/

        printf("Calibrating the camera with flipped samples...");
        fflush(stdout);
        calib::CameraCalibrator::calibrateCamera(camContainerFlip);
        printf("ok\n");

    }


    {

        printf("Calibrating the LEDs with flipped samples...\n");
        Camera camFlip;
        camFlip.setIntrinsicMatrix(camContainerFlip.intr);
        camFlip.setDistortion(camContainerFlip.dist);
        int nFlipLEDs = LEDContainersFlip.size();
        for(int i = 0; i < nFlipLEDs; ++i) {

            calib::calibrateLED(LEDContainersFlip[i], camFlip);

        }

    }


    printf("******************************************************************\n");
    printf("Original, not flipped\n");
    printCameraData(camContainer);
    printLEDData(LEDContainers);


    printf("******************************************************************\n");
    printf("New, flipped\n");
    printCameraData(camContainerFlip);
    printLEDData(LEDContainersFlip);


    CalibDataWriter writer;
    if(!writer.create(outputFile.c_str())) {
        printf("Could not create %s\n", outputFile.c_str());
        return -1;

    }

    if(!writer.writeCameraData(camContainerFlip)) {
        printf("Could not write the camera data into the xml file\n");
        return -1;
    }
    if(!writer.writeLEDData(LEDContainersFlip)) {
        printf("Could not write the LED data into the xml file\n");
        return -1;
    }

    printf("XML file %s written\n", outputFile.c_str());


    return 0;

}


void printCameraData(const calib::CameraCalibContainer &container) {


    printf("intr: \n");

    for(int row = 0; row < 3; ++row) {

        for(int col = 0; col < 3; ++col) {

            // transposed to get row-major!
            printf("%f ", container.intr[col*3 + row]);

        }

        printf("\n");

    }

    printf("\ndist: %f %f %f %f %f\n",
           container.dist[0],
           container.dist[1],
           container.dist[2],
           container.dist[3],
           container.dist[4]);

    printf("\nreprojection error %f\n", container.reproj_err);

}


void printLEDData(const std::vector<calib::LEDCalibContainer> &containers) {


}


void swapPoints(cv::Point2f &p1, cv::Point2f &p2) {

    cv::Point2f tmp = p1;

    p1 = p2;
    p2 = tmp;

}


/*
 *         15   14   13
 *  0  o    o    o    o    o 12
 *
 *  1  o                   o 11
 *
 *  2  o                   o 10
 *
 *  3  o                   o 9
 *
 *  4  o    o    o    o    o 8
 *          5    6    7
 */
void enumerateLEDImagePoints(std::vector<calib::LEDCalibContainer> &LEDContainers) {

    int nContainers = LEDContainers.size();

    for(int cContainer = 0; cContainer < nContainers; ++cContainer) {

        calib::LEDCalibContainer &curContainer = LEDContainers[cContainer];
        std::vector<calib::LEDCalibSample> &samples = curContainer.getSamples();

        int nSamples = samples.size();

        for(int cSample = 0; cSample < nSamples; ++cSample) {

            std::vector<cv::Point2f> &imagePoints = samples[cSample].image_points;

            swapPoints(imagePoints[0], imagePoints[12]);  // swap 0 and 12
            swapPoints(imagePoints[1], imagePoints[11]);  // swap 1 and 11
            swapPoints(imagePoints[2], imagePoints[10]);  // swap 2 and 10
            swapPoints(imagePoints[3], imagePoints[9]);   // swap 3 and 9
            swapPoints(imagePoints[4], imagePoints[8]);   // swap 4 and 8

            swapPoints(imagePoints[13], imagePoints[15]); // swap 13 and 15
            swapPoints(imagePoints[5], imagePoints[7]);   // swap 5 and 7

        }


    }


}


bool readCalibData(const std::string &calibFile,
                   calib::CameraCalibContainer &camContainer,
                   std::vector<calib::LEDCalibContainer> &LEDContainers) {

	/**********************************************************************
	 * Open the settings file and read the settings of the calib reader
	 *********************************************************************/
	CalibDataReader calibReader;
	if(!calibReader.create(calibFile)) {

        printf("Could not open %s\n", calibFile.c_str());

		return false;

	}


	/**********************************************************************
	 * Read the camera calibration container and assign the values to
	 * the camera
	 *********************************************************************/

    if(!calibReader.readCameraContainer(camContainer)) {

        printf("Could not read the camera container\n");

        return false;

    }


	/**********************************************************************
	 * Load the LED positions from the file and assign the to the pattern
	 *********************************************************************/

    if(!calibReader.readLEDContainers(LEDContainers)) {

        printf("Could not read the LED containers\n");

        return false;

    }

    if(LEDContainers.size() == 0) {

        printf("Read the LED containers, but there is no data...\n");

        return false;

    }


    return true;

}


bool handleInputParameters(int  argc, const char **args) {

    std::vector<ParamAndValue> argVec;
    if(!parseInput(argc, args, argVec)) {
        printf("handleInputParameters(): Error parsing input\n");
        return false;
    }


    for(int i = 0; i < (int)argVec.size(); ++i) {

        if(!handleParameter(argVec[i])) {

            printf("-%s %s not defined\n", argVec[i].name.c_str(), argVec[i].value.c_str());
            return false;

        }

    }


    return true;

}


bool handleParameter(const ParamAndValue &pair) {

    if(pair.name == "i") {

        inputFile = pair.value;

    }

    else if(pair.name == "o") {

        outputFile = pair.value;

    }

    else if(pair.name == "h" || pair.name == "help") {

        bPrintHelp = true;

    }

    else {
        return false;
    }


    return true;

}


void printUsageInfo() {

    printf("Usage:\n"
           "  ./calibrate [option arguments]\n"
           "  option arguments:\n"
           "      [-i <input_file>]    Must be defined. xml calibration input file, usually calibration.calib\n"
           "      [-o <output_file>]   Must be defined. xml calibration output file\n"
           "      [-h]                 Display help\n"
           "      [-help]              Same as -h\n"
           );

}


bool fileExists(const char *filename) {

    std::ifstream file(filename);
    return file.is_open();

}

