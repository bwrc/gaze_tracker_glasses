#include "CalibDataReader.h"
#include <list>


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


CalibDataReader::CalibDataReader() {

}


CalibDataReader::~CalibDataReader() {

}


bool CalibDataReader::create(const std::string &path) {

	bool bLoadOk = doc.LoadFile(path.c_str());


	if(!bLoadOk) {

		return false;

	}

	return true;

}


// struct sortLEDs {
//     bool operator() (calib::LEDCalibSample &s1, calib::LEDCalibSample &s2) {
//         return (s1.id < s2.id);
//     }
// } sortLEDs;

bool sortLEDs(calib::LEDCalibContainer s1, calib::LEDCalibContainer s2) {
    return (s1.id < s2.id);
}


void CalibDataReader::sortLEDContainersByID(std::vector<calib::LEDCalibContainer> &containers) {

    std::sort(containers.begin(), containers.end(), sortLEDs);

}


bool CalibDataReader::readCameraContainer(calib::CameraCalibContainer &container) {

	container.clear();


	// get the root element
	TiXmlNode *root = doc.RootElement();

	if(!root) {

		return false;

	}

	// get the first child
	TiXmlElement *elCamera = root->FirstChildElement(XML_ELEMENT_CAMERA);

	if(!elCamera) {

		return false;

	}

	/*******************************************************
	 * <camera> has been found, next look for:
	 *
	 *     <sample id="<number>">
	 *     <intr>
	 *     <dist>
	 *     <reproj_err>
	 *     <object_points>
	 *     <image_size>
	 *
	 *******************************************************/


	/*******************************************************
	 * Look for all <sample> elements
	 *******************************************************/

	if(!readCameraSamples(elCamera, container)) { // also checks for the validity of elSample
		return false;
	}


	/*******************************************************
	 * Look for <intr>
	 *******************************************************/

	TiXmlElement *elIntr = elCamera->FirstChildElement(XML_ELEMENT_INTR);

	if(elIntr) {

		const char *txt = elIntr->Value();
		if(txt == NULL) {
			return false;
		}

		const char *value = elIntr->GetText();
		if(value == NULL) {
			return false;
		}

		if(!CalibDataReader::strToVec(value, container.intr, 9)) {
			return false;
		}

	}


	/*******************************************************
	 * Look for <dist>
	 *******************************************************/

	TiXmlElement *elDist = elCamera->FirstChildElement(XML_ELEMENT_DIST);
	if(elDist) {

		const char *txt = elDist->Value();
		if(txt == NULL) {
			return false;
		}

		const char *value = elDist->GetText();
		if(value == NULL) {
			return false;
		}

		if(!CalibDataReader::strToVec(value, container.dist, 5)) {
			return false;
		}

	}


	/*******************************************************
	 * Look for <reproj_err>
	 *******************************************************/

	TiXmlElement *elReprojErr = elCamera->FirstChildElement(XML_ELEMENT_REPROJ_ERR);
	if(elReprojErr) {

		const char *txt = elReprojErr->Value();
		if(txt == NULL) {
			return false;
		}

		const char *value = elReprojErr->GetText();
		if(value == NULL) {
			return false;
		}

		bool ok;
		double err = CalibDataReader::toDouble(value, &ok);


		if(!ok) {
			return false;
		}

		container.reproj_err = err;

	}


	/*******************************************************
	 * Look for <object_points>
	 *******************************************************/

	TiXmlElement *elObjPoints = elCamera->FirstChildElement(XML_ELEMENT_OBJECT_POINTS);
	if(elObjPoints) {

		const char *txt = elObjPoints->Value();
		if(txt == NULL) {
			return false;
		}

		const char *value = elObjPoints->GetText();
		if(value == NULL) {
			return false;
		}

		std::vector<cv::Point3f> &object_points = container.object_points;

		if(!CalibDataReader::_3dPointStrToVec(value, object_points)) {

			return false;

		}

	}


	/*******************************************************
	 * Look for <image_size>
	 *******************************************************/

	TiXmlElement *elImgSize = elCamera->FirstChildElement(XML_ELEMENT_IMAGE_SIZE);
	if(elImgSize) {

		const char *txt = elImgSize->Value();
		if(txt == NULL) {
			return false;
		}

		const char *value = elImgSize->GetText();
		if(value == NULL) {
			return false;
		}

		double vec[2];
		if(!CalibDataReader::strToVec(value, vec, 2)) {
			return false;
		}

		container.imgSize.width  = vec[0];
		container.imgSize.height = vec[1];

	}

	return true;

}


bool CalibDataReader::readCameraSamples(TiXmlElement *elCamera,
								  calib::CameraCalibContainer &container) {

	TiXmlElement *elSample = elCamera->FirstChildElement(XML_ELEMENT_SAMPLE);

	while(elSample) {

		// get the first child of this sample
		TiXmlElement *elSampleChild = elSample->FirstChildElement();

		// the newsamples which will be inserted into the container on success.
		calib::CameraCalibSample newSample;


		while(elSampleChild) {

			/***********************************************************
			 * In <sample> there are:
			 *
			 *     <image_path>/home/sharman/.../cameraCalib1.jpg</image_path>
			 *     <image_points>(59,21),(3,4),(5,6),(7,8),(9,10)...</image_points>
			 *
			 ***********************************************************/

			/*
			 * Get the name and text of the element, a bit strange naming convention
			 * on tinyxml's part
			 */
			const char *ctxt = elSampleChild->Value();
			const std::string name(ctxt != NULL ? ctxt : "");

			ctxt = elSampleChild->GetText();
			const std::string value(ctxt != NULL ? ctxt : "");


			if(name == std::string(XML_ELEMENT_IMAGE_PATH)) {

				newSample.img_path = value;

			}
			else if(name == std::string(XML_ELEMENT_IMAGE_POINTS)) {

				if(!CalibDataReader::_2dPointStrToVec(value, newSample.image_points)) {

					return false;

				}

			}

			elSampleChild = elSampleChild->NextSiblingElement();

		}

		// get the next <sample>
		elSample = elSample->NextSiblingElement(XML_ELEMENT_SAMPLE);

		// add the sample into the container
		container.addSample(newSample);

	}


	return true;

}


bool CalibDataReader::readLEDContainers(std::vector<calib::LEDCalibContainer> &containers) {

	containers.clear();

	// get the root element
	TiXmlNode *root = doc.RootElement();

	// get the first child
	TiXmlElement *n = root->FirstChildElement();

	while(n) {

		const char *ctxt = n->Value();
		std::string name(ctxt ? ctxt : "");


		// see if <LEDs>
		if(name == std::string(XML_ELEMENT_LEDS)) {

			/*******************************************************
			 * <LEDS> has been found, next look for:
			 *
			 *     <LED id="<number>">
			 *
			 *******************************************************/

			/*******************************************************
			 * Look for a <LED> element
			 *******************************************************/

			TiXmlElement *elLED = n->FirstChildElement(XML_ELEMENT_LED);


			while(elLED) {

				// place an empty container
				containers.push_back(calib::LEDCalibContainer());

				// and get a reference to it
				calib::LEDCalibContainer &c = containers.back();

				if(!readLEDContainer(elLED, c)) {
					return false;
				}

                int id;
                elLED->QueryIntAttribute("id", &id);
                c.id = id;

				elLED = elLED->NextSiblingElement(XML_ELEMENT_LED);

			}

            sortLEDContainersByID(containers);

            // break the while-loop because <LEDs> has been handled
            break;

		}

		n = n->NextSiblingElement();

	}

	return true;

}


bool CalibDataReader::readLEDContainer(TiXmlElement *elLED, calib::LEDCalibContainer &container) {

	/***********************************************************************
	 * An example of the contents of <LED id="someid">
	 *
	 *     <led_pos>0.0,0.0,0.0</led_pos>
	 *     <circle_spacing>1.200000</circle_spacing>
	 *     <object_points>(1,2,3),(1,2,3),(1,2,3),(1,2,3)</object_points>
	 *
	 *     <sample id="1">
	 *          <image_path>/home/sharman/.../LEDCalib1.jpg</image_path>
	 *          <circle_spacing>1.2</circle_spacing>
	 *          <image_points>(1,2),(1,2),(1,2),(1,2)</image_points>
	 *          <glint_pos>(449.0,279.0)</glint_pos>
	 *     </sample>
	 *
	 *     <sample id="2">
	 *        	<image_path>/home/sharman/.../LEDCalib2.jpg</image_path>
	 *        	<circle_spacing>1.2</circle_spacing>
	 *        	<image_points>(1,2),(1,2),(1,2),(1,2)</image_points>
	 *          <glint_pos>(449.0,279.0)</glint_pos>
	 *     </sample>
	 *
	 ***********************************************************************/

	container.clear();

	/*******************************************************
	 * Look for <led_pos>
	 *******************************************************/

	TiXmlElement *elLEDPos = elLED->FirstChildElement(XML_ELEMENT_LED_POS);
	if(!elLEDPos) {
		return false;
	}

	const char *value = elLEDPos->GetText();
	if(!value) {
		return false;
	}

	if(!CalibDataReader::strToVec(value, container.LED_pos, 3)) {
		return false;
	}


	/*******************************************************
	 * Look for <circle_spacing>
	 *******************************************************/

	TiXmlElement *elCS = elLED->FirstChildElement(XML_ELEMENT_CIRCLE_SPACING);
	if(!elCS) {
		return false;
	}

	value = elCS->GetText();
	if(!value) {
		return false;
	}

	bool ok;
	double cs = CalibDataReader::toDouble(value, &ok);
	if(!ok) {
		return false;
	}
	container.circleSpacing = cs;


	/*******************************************************
	 * Look for <object_points>
	 *******************************************************/

	TiXmlElement *elObjPoints = elLED->FirstChildElement(XML_ELEMENT_OBJECT_POINTS);
	if(!elObjPoints) {
		return false;
	}

	value = elObjPoints->GetText();
	if(!value) {
		return false;
	}

	std::vector<cv::Point3f> &normalisedObjectPoints = container.normalisedObjectPoints;
	if(!_3dPointStrToVec(value, normalisedObjectPoints)) {

		return false;

	}


	/*******************************************************
	 * Look for <sample>
	 *******************************************************/

	TiXmlElement *elSample = elLED->FirstChildElement(XML_ELEMENT_SAMPLE);
	if(elSample) {

		if(!readLEDSamples(elSample, container)) {
			return false;
		}

	}

	return true;


}


bool CalibDataReader::readLEDSamples(TiXmlElement *elSample, calib::LEDCalibContainer &container) {

	while(elSample) {

		calib::LEDCalibSample newSample;

		// get the first child of this sample
		TiXmlElement *elSampleChild = elSample->FirstChildElement();

		while(elSampleChild) {

			/***********************************************************
			 * In <sample> there are:
			 *
			 * <image_path>/home/sharman/.../LEDCalib1.jpg</image_path>
			 * <image_points>(1,2),(1,2),(1,2),(1,2)</image_points>
			 * <glint_pos>449.0,279.0</glint_pos>
			 *
			 ***********************************************************/

			// get the text of the element
			const char *ctxt = elSampleChild->Value();
			std::string name(ctxt ? ctxt : "");
			ctxt = elSampleChild->GetText();
			std::string value(ctxt ? ctxt : "");

			if(name == std::string(XML_ELEMENT_IMAGE_PATH)) {

				newSample.img_path = value;

			}
			else if(name == std::string(XML_ELEMENT_IMAGE_POINTS)) {

				if(!_2dPointStrToVec(value, newSample.image_points)) {

					return false;

				}

			}
			else if(name == std::string(XML_ELEMENT_GLINT_POS)) {

				double vec[2];

				if(!CalibDataReader::strToVec(value, vec, 2)) {

					return false;

				}

				newSample.glint.x = vec[0];
				newSample.glint.y = vec[1];

			}

			elSampleChild = elSampleChild->NextSiblingElement();

		}


		container.addSample(newSample);

		// get the next <sample>
		elSample = elSample->NextSiblingElement(XML_ELEMENT_SAMPLE);

	}

	return true;

}


void CalibDataReader::split(const std::string &str, std::vector<std::string> &strList, const char delim) {

	// clear the list first
	strList.clear();

	size_t pos = 0;
	while(pos != std::string::npos) {

		int old_pos = pos == 0 ? -1 : pos;
		pos = str.find(delim, pos + 1);

		// inclusive bounds
		int b1 = old_pos + 1;
		int b2 = pos - 1;

		int len = b2 - b1 + 1;

		strList.push_back(str.substr(b1, len));

	}

}


void CalibDataReader::split(const std::string &str, std::list<std::string> &strList, const char delim) {

	// clear the list first
	strList.clear();

	size_t pos = 0;
	while(pos != std::string::npos) {

		int old_pos = pos == 0 ? -1 : pos;
		pos = str.find(delim, pos + 1);

		// inclusive bounds
		int b1 = old_pos + 1;
		int b2 = pos - 1;

		int len = b2 - b1 + 1;

		strList.push_back(str.substr(b1, len));

	}

}



bool CalibDataReader::strToVec(const std::string &str, double *vec, int n) {

	std::list<std::string> strList;
	CalibDataReader::split(str, strList, ',');

	int sz = (int)strList.size();

	// if the list contains fewer elements than required, something has gone wrong
	if(sz != n) {
		return false;
	}

	std::list<std::string>::iterator it  = strList.begin();
	std::list<std::string>::iterator end = strList.end();

	while(it != end) {

		bool ok;
		double val = CalibDataReader::toDouble(it->c_str(), &ok);

		if(!ok) {
			return false;
		}

		*vec++ = val;

		++it;

	}

	return true;

}


bool CalibDataReader::strToVec(const std::string &str, float * vec, int n) {

	std::list<std::string> strList;
	CalibDataReader::split(str, strList, ',');

	// if the list contains fewer elements than required, something has gone wrong
	if((int)strList.size() != n) {
		return false;
	}

	std::list<std::string>::iterator it  = strList.begin();
	std::list<std::string>::iterator end = strList.end();

	while(it != end) {

		bool ok;
		double val = CalibDataReader::toDouble(it->c_str(), &ok);

		if(!ok) {
			return false;
		}

		*vec++ = (float)val;

		++it;

	}

	return true;

}


double CalibDataReader::toDouble(const char *str, bool *ok) {

	char *endptr;
	double val = strtod(str, &endptr);

	// check if the pointer points to a non-numerical character
	if(strcmp(endptr, "") != 0) {

		*ok = false;
		return 0.0;

	}

	*ok = true;
	return val;

}


static char cremove = ' ';

bool f1(char c) {
	return c == cremove;
}

void CalibDataReader::remove(std::string &str, char c) {
	cremove = c;
	str.erase(std::remove_if(str.begin(), str.end(), f1), str.end());

}


bool CalibDataReader::_2dPointStrToVec(const std::string &str_image_points, std::vector<cv::Point2f> &image_points) {

	image_points.clear();

	// copy string, so operations can be applied
	std::string str = str_image_points;

	/*
	 * "(1,2),(3,2),(1,1)"
	 *        =>
	 * "1,2,3,2,1,1"
	 *       =>
	 * ["1", "2", "3", "2", "1", "1"]
	 */
	CalibDataReader::remove(str, '(');
	CalibDataReader::remove(str, ')');

	std::vector<std::string> strList;
	CalibDataReader::split(str, strList, ',');


	// size must be even
	if(strList.size() % 2 != 0) {
		return false;
	}


	int nPoints = strList.size() / 2;
	image_points.resize(nPoints);

	for(int i = 0; i < nPoints; ++i) {

		bool ok;
		double x = CalibDataReader::toDouble(strList[2*i].c_str(), &ok);

		if(!ok) {
			return false;
		}

		double y = CalibDataReader::toDouble(strList[2*i+1].c_str(), &ok);

		if(!ok) {
			return false;
		}

		image_points[i] = cv::Point2f(x, y);

	}


	return true;

}


bool CalibDataReader::_3dPointStrToVec(const std::string &str_object_points, std::vector<cv::Point3f> &object_points) {

	object_points.clear();

	// copy string, so operations can be applied
	std::string str = str_object_points;


	/*
	 * "(1,2,3),(3,2,4),(1,1,5)"
	 *        =>
	 * "1,2,3,3,2,4,1,1,5"
	 *       =>
	 * ["1", "2", "3", "3", "2", "4", "1", "1", "5"]
	 */
	CalibDataReader::remove(str, '(');
	CalibDataReader::remove(str, ')');

	std::vector<std::string> strList;
	CalibDataReader::split(str, strList, ',');

	// size must be divisible by 3
	if(strList.size() % 3 != 0) {
		return false;
	}


	int nPoints = strList.size() / 3;
	object_points.resize(nPoints);

	for(int i = 0; i < nPoints; ++i) {

		bool ok;
		double x = CalibDataReader::toDouble(strList[3*i].c_str(), &ok);

		if(!ok) {
			return false;
		}

		double y = CalibDataReader::toDouble(strList[3*i+1].c_str(), &ok);

		if(!ok) {
			return false;
		}

		double z = CalibDataReader::toDouble(strList[3*i+2].c_str(), &ok);

		if(!ok) {
			return false;
		}

		object_points[i] = cv::Point3f(x, y, z);

	}


	return true;

}

