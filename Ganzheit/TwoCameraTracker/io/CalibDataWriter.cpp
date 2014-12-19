#include "CalibDataWriter.h"
#include <algorithm>
#include <iostream>
#include <limits>


static const char *XML_DOCUMENT_TYPE			= "CALIBRATION";
static const char *XML_ROOT_TAG_NAME			= "calibration";
static const char *XML_ELEMENT_CAMERA			= "camera";
static const char *XML_ELEMENT_LEDS				= "LEDs";
static const char *XML_ELEMENT_LED				= "LED";
static const char *XML_ELEMENT_IMAGE_PATH		= "image_path";
static const char *XML_ELEMENT_CIRCLE_SPACING	= "circle_spacing";
static const char *XML_ELEMENT_IMAGE_POINTS		= "image_points";
static const char *XML_ELEMENT_OBJECT_POINTS	= "object_points";
static const char *XML_ELEMENT_GLINT_POS		= "glint_pos";
static const char *XML_ELEMENT_INTR				= "intr";
static const char *XML_ELEMENT_DIST				= "dist";
static const char *XML_ELEMENT_REPROJ_ERR		= "reprojection_error";
static const char *XML_ELEMENT_LED_POS			= "led_pos";
static const char *XML_ELEMENT_SAMPLE			= "sample";
static const char *XML_ELEMENT_IMAGE_SIZE		= "image_size";
static const int PRINT_PRECISION_DBL            = std::numeric_limits<double>::digits10;




static void vecTo2dPointStr(const std::vector<cv::Point2f> &image_points, std::string &str) {

	str.clear();

	size_t sz = image_points.size();


    std::stringstream ss;
    ss.precision(PRINT_PRECISION_DBL);

	for(size_t i = 0; i < sz; ++i) {

		const cv::Point2f &p = image_points[i];

        ss << "(" << p.x << "," << p.y << "),";

	}

    str = ss.str();

	// remove the last ,
	if(str.length() > 1) {

		str.resize(str.length() - 1);

	}

}


static void vecTo3dPointStr(const std::vector<cv::Point3f> &object_points, std::string &str_object_points) {

	str_object_points.clear();

	size_t sz = object_points.size();

    std::stringstream ss;
    ss.precision(PRINT_PRECISION_DBL);

	for(size_t i = 0; i < sz; ++i) {

		const cv::Point3f &p = object_points[i];

        ss << "(" << p.x << "," << p.y << "," << p.z << "),";

	}

    str_object_points = ss.str();

	// remove the last ,
	if(str_object_points.length() > 1) {

		str_object_points.resize(str_object_points.length() - 1);

	}

}


CalibDataWriter::CalibDataWriter() {

	root = NULL;

}


CalibDataWriter::~CalibDataWriter() {

    // will cause an error, if doc deletes this too
    //    delete root;

    close();

}


bool CalibDataWriter::create(const std::string &path) {

    saveFile = path;

    root = new TiXmlElement(XML_ROOT_TAG_NAME);

    doc.LinkEndChild(root);

	return true;

}


void CalibDataWriter::close() {

	doc.SaveFile(saveFile.c_str());

}


/*
 *
 *	<camera>
 *
 *		<object_points>(19,31,43),(3,4,5),(5,6,7),(7,8,9),(9,10,11)</object_points>
 *		<intr>751.778295,0.000000,318.560908,0.000000,754.591145,229.429121,0.000000,0.000000,1.000000</intr>
 *		<dist>0.210905,-0.561128,-0.001850,-0.001595,-2.703326</dist>
 *		<reprojection_error>0.123750</reprojection_error>
 *		<image_size>640,480</image_size>
 *
 *		<sample id="1">
 *			<image_path>/home/sharman/.../cameraCalib1.jpg</image_path>
 *			<image_points>(59,21),(3,4),(5,6),(7,8),(9,10)</image_points>
 *		</sample>
 *
 *
 *		<sample id="2">
 *			<image_path>/home/sharman/.../cameraCalib2.jpg</image_path>
 *			<image_points>(59,21),(3,4),(5,6),(7,8),(9,10)</image_points>
 *		</sample>
 *
 *
 *		<sample id="3">
 *			<image_path>/home/sharman/.../cameraCalib3.jpg</image_path>
 *			<image_points>(59,21),(3,4),(5,6),(7,8),(9,10)</image_points>
 *		</sample>
 *
 *	</camera>
 *
 */
bool CalibDataWriter::writeCameraData(const calib::CameraCalibContainer &cameraCalib) {

	// create the camera element
	TiXmlElement *elCamera = new TiXmlElement(XML_ELEMENT_CAMERA);
    root->LinkEndChild(elCamera);

	/******************************************************************
	 * Write the results
	 ******************************************************************/

	// intrinsic matrix element
	{

		TiXmlElement *newEl = new TiXmlElement(XML_ELEMENT_INTR);
		elCamera->LinkEndChild(newEl);

        std::string str;
		CalibDataWriter::vecToStr(cameraCalib.intr, 9, str);
		TiXmlText *newNodeText = new TiXmlText(str.c_str());
		newEl->LinkEndChild(newNodeText);

	}

	// distortion vector element
	{

		TiXmlElement *newEl = new TiXmlElement(XML_ELEMENT_DIST);
		elCamera->LinkEndChild(newEl);

        std::string str;
		CalibDataWriter::vecToStr(cameraCalib.dist, 5, str);
		TiXmlText *newNodeText = new TiXmlText(str.c_str());
		newEl->LinkEndChild(newNodeText);

	}

	// error element
	{

		TiXmlElement *newEl = new TiXmlElement(XML_ELEMENT_REPROJ_ERR);
		elCamera->LinkEndChild(newEl);

        char str[128];
		sprintf(str, "%f", cameraCalib.reproj_err);
		TiXmlText *newNodeText = new TiXmlText(str);
		newEl->LinkEndChild(newNodeText);

	}

	// object points
	{

		TiXmlElement *elObjPoints = new TiXmlElement(XML_ELEMENT_OBJECT_POINTS);
		elCamera->LinkEndChild(elObjPoints);

        std::string str_obj_points;
		vecTo3dPointStr(cameraCalib.object_points, str_obj_points);
		TiXmlText *domxt = new TiXmlText(str_obj_points.c_str());
		elObjPoints->LinkEndChild(domxt);

	}

	// image size
	{
		TiXmlElement *elImgSize = new TiXmlElement(XML_ELEMENT_IMAGE_SIZE);
		elCamera->LinkEndChild(elImgSize);

        std::stringstream ss;
        ss.precision(PRINT_PRECISION_DBL);
		ss << cameraCalib.imgSize.width << "," << cameraCalib.imgSize.height;
		TiXmlText *domxt = new TiXmlText(ss.str().c_str());
		elImgSize->LinkEndChild(domxt);

	}

	const std::vector<calib::CameraCalibSample> &samples = cameraCalib.getSamples();

	// sanity check
	if(samples.size() < 1) {

		return false;

	}


	/******************************************************************
	 * Write the samples, Example:
	 *
	 *	<sample id="3">
	 *		<image_path>/home/sharman/.../cameraCalib3.jpg</image_path>
	 *		<image_points>(59,21),(3,4),(5,6),(7,8),(9,10)...</image_points>
	 *	</sample>
	 *
	 ******************************************************************/

	size_t sz = samples.size();

	for(size_t i = 0; i < sz; ++i) {

		TiXmlElement *elSample = new TiXmlElement(XML_ELEMENT_SAMPLE);
        elCamera->LinkEndChild(elSample);

		// set id attribute
		elSample->SetAttribute("id", i);

		// image path
		TiXmlElement *elImgPath = new TiXmlElement(XML_ELEMENT_IMAGE_PATH);
        elSample->LinkEndChild(elImgPath);

		TiXmlText *domxt = new TiXmlText(samples[i].img_path.c_str());
		elImgPath->LinkEndChild(domxt);


		// image points
		TiXmlElement *elImgPoints = new TiXmlElement(XML_ELEMENT_IMAGE_POINTS);
		elSample->LinkEndChild(elImgPoints);

        std::string str_image_points;
		vecTo2dPointStr(samples[i].image_points, str_image_points);
		domxt = new TiXmlText(str_image_points.c_str());
		elImgPoints->LinkEndChild(domxt);

	}

	return true;

}


/*
 *	<LEDs>
 *
 *		<LED id="1">
 *
 *			<led_pos>0.0,0.0,0.0</led_pos>
 *			<circle_spacing>1.2</circle_spacing>
 *			<object_points>(1,2,3),(1,2,3),(1,2,3),(1,2,3)</object_points>
 *
 *			<sample id="1">
 *				<image_path>/home/sharman/.../LEDCalib1.jpg</image_path>
 *				<image_points>(1,2),(1,2),(1,2),(1,2)</image_points>
 *			</sample>
 *
 *			<sample id="2">
 *				<image_path>/home/sharman/.../LEDCalib2.jpg</image_path>
 *				<image_points>(1,2),(1,2),(1,2),(1,2)</image_points>
 *			</sample>
 *
 *		</LED>
 *
 *
 *		<LED id="2">
 *
 *			<led_pos>0.0,0.0,0.0</led_pos>
 *			<circle_spacing>1.2</circle_spacing>
 *			<object_points>(1,2,3),(1,2,3),(1,2,3),(1,2,3)</object_points>
 *
 *			<sample id="1">
 *				<image_path>/home/sharman/.../LEDCalib3.jpg</image_path>
 *				<image_points>(1,2),(1,2),(1,2),(1,2)</image_points>
 *			</sample>
 *
 *			<sample id="2">
 *				<image_path>/home/sharman/.../LEDCalib4.jpg</image_path>
 *				<image_points>(1,2),(1,2),(1,2),(1,2)</image_points>
 *			</sample>
 *
 *			<sample id="3">
 *				<image_path>/home/sharman/.../LEDCalib5.jpg</image_path>
 *				<image_points>(1,2),(1,2),(1,2),(1,2)</image_points>
 *			</sample>
 *
 *		</LED>
 *
 *	</LEDs>
 *
 */
bool CalibDataWriter::writeLEDData(const std::vector<calib::LEDCalibContainer> &LEDData) {

	size_t sz = LEDData.size();

	// create the LEDs element
	TiXmlElement *elLEDs = new TiXmlElement(XML_ELEMENT_LEDS);
	root->LinkEndChild(elLEDs);


	for(size_t i = 0; i < sz; ++i) {

		// get the current container
		const calib::LEDCalibContainer &container = LEDData[i];


		// create a new LED element
		TiXmlElement *elLED = new TiXmlElement(XML_ELEMENT_LED);
		elLEDs->LinkEndChild(elLED);

		// set id attribute
		elLED->SetAttribute("id", i);

		/*****************************************************************
		 * LED position element
		 *****************************************************************/
		TiXmlElement *elLEDPos = new TiXmlElement(XML_ELEMENT_LED_POS);
		elLED->LinkEndChild(elLEDPos);

        std::string str;
		CalibDataWriter::vecToStr(container.LED_pos, 3, str);
		TiXmlText *domtxt = new TiXmlText(str.c_str());
		elLEDPos->LinkEndChild(domtxt);


		/*****************************************************************
		 * Circle spacing position element
		 *****************************************************************/
		TiXmlElement *elCS = new TiXmlElement(XML_ELEMENT_CIRCLE_SPACING);
		elLED->LinkEndChild(elCS);

        char cstr[128];
		sprintf(cstr, "%f", container.circleSpacing);
		domtxt = new TiXmlText(cstr);
		elCS->LinkEndChild(domtxt);


		/*****************************************************************
		 * Object points element
		 *****************************************************************/
		TiXmlElement *elObjPoints = new TiXmlElement(XML_ELEMENT_OBJECT_POINTS);
		elLED->LinkEndChild(elObjPoints);

        std::string str_obj_points;
		vecTo3dPointStr(container.normalisedObjectPoints, str_obj_points);
		domtxt = new TiXmlText(str_obj_points.c_str());
		elObjPoints->LinkEndChild(domtxt);


		/*****************************************************************
		 * Samples, example:
		 *
		 *	<sample id="3">
		 *		<image_path>/home/sharman/.../LEDCalib5.jpg</image_path>
		 *		<image_points>(1,2),(1,2),(1,2),(1,2)...</image_points>
		 *		<glint_pos>56,90</glint_pos>
		 *	</sample>
		 *
		 *****************************************************************/

		// get the samples
		const std::vector<calib::LEDCalibSample> &samples = container.getSamples();


		// number of samples
		size_t numSamples = samples.size();

		for(size_t j = 0; j < numSamples; ++j) {

			// sample element
			TiXmlElement *elSample = new TiXmlElement(XML_ELEMENT_SAMPLE);
			elLED->LinkEndChild(elSample);

			// put the "id" attribute
			elSample->SetAttribute("id", j);

			// image path
			TiXmlElement *elImgPath = new TiXmlElement(XML_ELEMENT_IMAGE_PATH);
			elSample->LinkEndChild(elImgPath);

			TiXmlText *domtxt = new TiXmlText(samples[j].img_path.c_str());
			elImgPath->LinkEndChild(domtxt);


			// image points
			TiXmlElement *elImgPoints = new TiXmlElement(XML_ELEMENT_IMAGE_POINTS);
			elSample->LinkEndChild(elImgPoints);

            std::string str_image_points;
			vecTo2dPointStr(samples[j].image_points, str_image_points);
			domtxt = new TiXmlText(str_image_points.c_str());
			elImgPoints->LinkEndChild(domtxt);


			// glint position
			TiXmlElement *elGlintPos = new TiXmlElement(XML_ELEMENT_GLINT_POS);
			elSample->LinkEndChild(elGlintPos);

            std::stringstream ss;
            ss.precision(PRINT_PRECISION_DBL);
            ss << samples[j].glint.x << "," << samples[j].glint.y;
			domtxt = new TiXmlText(ss.str().c_str());
			elGlintPos->LinkEndChild(domtxt);

		}

	}

	return true;

}


void CalibDataWriter::vecToStr(const double *vec, int n, std::string &oput) {

    std::stringstream ss;
    ss.precision(PRINT_PRECISION_DBL);

	for(int i = 0; i < n; ++i) {

        ss << vec[i] << ",";

	}

    oput = ss.str();

	// remove the last ','
	oput.resize(oput.length() - 1);

}

