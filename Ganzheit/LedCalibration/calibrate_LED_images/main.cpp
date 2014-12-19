#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "Camera.h"

#include "LEDCalibrator.h"
#include "LEDCalibPattern.h"
#include "CalibDataReader.h"
#include "CalibDataWriter.h"

#include "InputParser.h"
#include <sys/time.h>




static const double MAX_FONT_SCALE = 2.6;

class TextAnimator {

public:

    enum {
        DUR_ANIMATION_MS = 1000
    };

    TextAnimator() {

        reset();

    }

    void reset() {

        gettimeofday(&tThen, NULL);
        fontScale = MAX_FONT_SCALE;

    }

    void setText(const std::string &_txt) {

        reset();

        txt = _txt;

    }

    void update(cv::Mat &imgBgr) {

        struct timeval tNow;
        gettimeofday(&tNow, NULL);
        long dSec = tNow.tv_sec - tThen.tv_sec;
        long dUsec = tNow.tv_usec - tThen.tv_usec;

        double millis = 1000.0 * dSec + dUsec / 1000.0;

        int thickness = 3;

        if(millis <= DUR_ANIMATION_MS) {

            int baseline = 0;
            cv::Size sz = cv::getTextSize(txt,
                                          cv::FONT_HERSHEY_SIMPLEX,
                                          fontScale,
                                          thickness,
                                          &baseline);

            cv::Point pointCentre((imgBgr.cols - sz.width)  / 2,
                                  imgBgr.rows - sz.height - 20);
        
            cv::putText(imgBgr,
                        txt,
                        pointCentre,
                        cv::FONT_HERSHEY_SIMPLEX,
                        fontScale,
                        cv::Scalar(0, 0, 255),
                        thickness,
                        CV_AA);

            fontScale = MAX_FONT_SCALE * (DUR_ANIMATION_MS - millis) / DUR_ANIMATION_MS;

        }

    }

private:

    struct timeval tThen;

    int maxCount;

    double fontScale;
    std::string txt;

};



/******************************************************************************
 * Prototypes
 ******************************************************************************/
static bool fileExists(const char *filename);
static void handleKeyboard(int key);
static bool handleInputParameters(int  argc, const char **args);
static bool handleParameter(const ParamAndValue &pair);
static void printUsageInfo();
static void draw(cv::Mat &img, const calib::LEDCalibSample &sample);
static bool readCalibData(const std::string &calibFile,
                          calib::CameraCalibContainer &camContainer,
                          std::vector<calib::LEDCalibContainer> &LEDContainers);

bool writeCalibData(const calib::CameraCalibContainer &camContainer,
                    const std::vector<calib::LEDCalibContainer> &LEDContainers);

void incContainer(int inc);
void nextContainer();
void prevContainer();

void deleteSample();
void incSample(int inc);
void nextSample();
void prevSample();

void onMouse(int evt, int x, int y, int flags, void* param);


static const int MODE_SAMPLES = 0;
static const int MODE_GLINT   = 1;
int mode = MODE_SAMPLES;


/******************************************************************************
 * Globals
 ******************************************************************************/
bool bUpdateGraphics = true;

TextAnimator animator;

std::vector<calib::LEDCalibContainer> LEDContainers;

int indContainer = 0;
int indSample    = 0;

calib::LEDCalibContainer *curContainer = NULL;
calib::LEDCalibSample *curSample = NULL;


// main loop state
static bool b_running = true;

static bool bPrintHelp = false;

std::string inputFile;
std::string outputFile;


int main(int argc, const char **args) {

	if(argc < 2) {

        printUsageInfo();

        return EXIT_FAILURE;

	}

    if(!handleInputParameters(argc, args)) {

        printUsageInfo();

        return EXIT_FAILURE;

    }

    if(bPrintHelp) {

        printUsageInfo();

        return EXIT_SUCCESS;

    }


    if(inputFile.empty()) {

        printf("-i <input_file> must be defined\n");

        printUsageInfo();

        return EXIT_FAILURE;

    }

    /*
     * Read the contents of the XML file
     */
    calib::CameraCalibContainer camContainer;
    if(!readCalibData(inputFile.c_str(), camContainer, LEDContainers)) {

        return EXIT_FAILURE;

    }


    /*
     * Not interested in empty containers.
     */
    if(LEDContainers.size() == 0) {

        printf("No LED containers\n");

        return EXIT_FAILURE;

    }

    if(LEDContainers[0].getSamples().size() == 0) {

        printf("Empty LED container\n");

        return EXIT_FAILURE;

    }

    if(camContainer.getSamples().size() == 0) {

        printf("Empty Cam container\n");

        return EXIT_FAILURE;

    }



	/**********************************************************************
	 * Initialize the main window
	 *********************************************************************/

	const std::string strWinSample("Sample window");
    cv::namedWindow(strWinSample, CV_WINDOW_AUTOSIZE);
    cvSetMouseCallback(strWinSample.c_str(), onMouse, 0);

    animator.setText("Start");

	/**********************************************************************
	 * Main loop
	 *********************************************************************/
    cv::Mat img;
	while(b_running) {

		// analyse and draw the results
        if(bUpdateGraphics) {

            const calib::LEDCalibSample &sample = LEDContainers[indContainer].getSamples()[indSample];
            draw(img, sample);
            bUpdateGraphics = false;

        }

        cv::Mat imgRender;
        img.copyTo(imgRender);

        animator.update(imgRender);

        int key = cv::waitKey(3);

        handleKeyboard(key);

        // show the frames
        cv::imshow(strWinSample, imgRender);

	}


    if(!outputFile.empty()) {

        if(!writeCalibData(camContainer, LEDContainers)) {

            return EXIT_FAILURE;

        }

    }


	return EXIT_SUCCESS;

}


void draw(cv::Mat &img, const calib::LEDCalibSample &sample) {

    img = cv::imread(sample.img_path.c_str());

    const std::vector<cv::Point2f> &imagePoints = sample.image_points;

    int sz = imagePoints.size();
    for(int i = 0; i < sz; ++i) {

        cv::Point c((int)(imagePoints[i].x + 0.5),
                    (int)(imagePoints[i].y + 0.5));

        cv::circle(img,
                   c,
                   3,
                   cv::Scalar(255, 0, 0),
                   2,
                   CV_AA);

        std::stringstream ss;
        ss << i;
        cv::putText(img,
                    ss.str(),
                    c,
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    cv::Scalar(255, 0, 255),
                    1,
                    CV_AA);

    }

    cv::Point c = cv::Point((int)(sample.glint.x + 0.5),
                            (int)(sample.glint.y + 0.5));

    cv::circle(img,
               c,
               3,
               cv::Scalar(0, 0, 255),
               CV_FILLED,
               CV_AA);

}


void handleKeyboard(int key) {

    char cKey = (char)key;

    // ESC
	if(cKey == 27) {
		b_running = false;
        return;
	}


    // toggle mode
    if(cKey == 'm') {

        mode = (mode + 1) % 2;

        std::stringstream ss;
        ss << "mode: " << (mode == MODE_SAMPLES ? "samples" : "glint");
        animator.setText(ss.str());

        return;

    }


    if(cKey == 'd') { // delete sample

        deleteSample();
        bUpdateGraphics = true;

    }


    // see which mode
    if(mode == MODE_SAMPLES) {

        if(cKey == 'S') { // right arrow

            nextSample();
            bUpdateGraphics = true;

        }
        else if(cKey == 'Q') { // left arrow

            prevSample();
            bUpdateGraphics = true;

        }
        else if(cKey == 'R') { // up arrow

            nextContainer();
            bUpdateGraphics = true;

        }
        else if(cKey == 'T') { // down arrow

            prevContainer();
            bUpdateGraphics = true;

        }

    }
    else if(mode == MODE_GLINT) {

        calib::LEDCalibSample &sample = LEDContainers[indContainer].getSamples()[indSample];

        if(cKey == 'S') { // right arrow

            int sugg = sample.glint.x + 1;
            if(sugg < 640) {
                sample.glint.x = sugg;
            }

            bUpdateGraphics = true;

        }
        else if(cKey == 'Q') { // left arrow

            int sugg = sample.glint.x - 1;
            if(sugg >= 0) {
                sample.glint.x = sugg;
            }

            bUpdateGraphics = true;

        }
        else if(cKey == 'R') { // up arrow

            int sugg = sample.glint.y - 1;
            if(sugg >= 0) {
                sample.glint.y = sugg;
            }

            bUpdateGraphics = true;

        }
        else if(cKey == 'T') { // down arrow

            int sugg = sample.glint.y + 1;
            if(sugg < 480) {
                sample.glint.y = sugg;
            }

            bUpdateGraphics = true;

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
	 * Load the LED positions from the file
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


bool writeCalibData(const calib::CameraCalibContainer &camContainer,
                    const std::vector<calib::LEDCalibContainer> &LEDContainers) {

    if(fileExists(outputFile.c_str())) {

        std::string input;
        printf("************************************************************\n");
        printf("File %s exists, overwrite? (yes/no)\n", outputFile.c_str());
        printf("************************************************************\n");

        while(input != "yes" && input != "no") {

            std::getline(std::cin, input);

            if(input == "no") {

                printf("Run the program and change the -o option\n");

                return false;

            }
            else if(input == "yes") {

                printf("overwriting %s\n", outputFile.c_str());

                break;

            }

        }

    }

    CalibDataWriter writer;
    if(!writer.create(outputFile.c_str())) {
        printf("Could not create %s\n", outputFile.c_str());
        return false;

    }
    if(!writer.writeCameraData(camContainer)) {
        printf("Could not write the camera data into the xml file\n");
        return false;
    }
    if(!writer.writeLEDData(LEDContainers)) {
        printf("Could not write the LED data into the xml file\n");
        return false;
    }

    printf("XML file %s written\n", outputFile.c_str());

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


void incContainer(int inc) {

    int nContainers = LEDContainers.size();

    int sugg = indContainer + inc;

    if(sugg < nContainers && sugg >= 0) {

        indContainer = sugg;

        std::stringstream ss;
        ss << "Container " << sugg + 1 << " / " << nContainers;

        animator.setText(ss.str());

        indSample = 0;

    }

}


void nextContainer() {

    incContainer(1);

}


void prevContainer() {

    incContainer(-1);

}


void incSample(int inc) {

    const calib::LEDCalibContainer &curContainer      = LEDContainers[indContainer];
    const std::vector<calib::LEDCalibSample> &samples = curContainer.getSamples();

    int nSamples = samples.size();

    int sugg = indSample + inc;
    if(sugg >= 0 && sugg < nSamples) {

        indSample = sugg;

        std::stringstream ss;
        ss << "Sample " << sugg + 1 << " / " << nSamples;

        animator.setText(ss.str());

    }

}


void deleteSample() {

    calib::LEDCalibContainer &curContainer = LEDContainers[indContainer];
    std::vector<calib::LEDCalibSample> &samples  = curContainer.getSamples();

    if(samples.size() == 1) {

        animator.setText("cannot delete only sample");
        return;

    }

    samples.erase(samples.begin() + indSample);

    if(indSample >= samples.size()) {
        --indSample;
    }

    animator.setText("deleted");

}



void nextSample() {

    incSample(1);

}


void prevSample() {

    incSample(-1);

}


void onMouse(int evt, int x, int y, int flags, void* param) {

    if(evt == CV_EVENT_LBUTTONDOWN) {

        if(mode == MODE_GLINT) {

            calib::LEDCalibSample &sample = LEDContainers[indContainer].getSamples()[indSample];

            sample.glint.x = x;
            sample.glint.y = y;

            bUpdateGraphics = true;

        }
        else {

            animator.setText("Switch to glint mode \"m\"");

        }

    }

}


void printUsageInfo() {

    printf("Usage:\n"
           "  ./calibrate [option arguments]\n"
           "  option arguments:\n"
           "      [-i <input_file>]    Input calibration XML file\n"
           "      [-o <output_file>]   Output calibration XML file\n"
           "      [-h]                 Display help\n"
           "      [-help]              Same as -h\n"
           );

    printf("  Controls:\n"
           "      m:             Change mode, change samples or move glint\n"
           "      d:             Delete the current sample. Fails if there is only one sample\n"
           "      left arrow:    Previous sample or move glint to the left\n"
           "      right arrow:   Next sample or move glint to the right\n"
           "      down arrow:    Previous container or move glint downwards\n"
           "      up arrow:      Next container or move glint upwards\n"
           "      ESC:           Quit program\n"
           );

}


bool fileExists(const char *filename) {

    std::ifstream file(filename);
    return file.is_open();

}


