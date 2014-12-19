#include "Settings.h"
#include <vector>


/*
 * 1. Output files
 * 2. Input devices
 * 3. Gazetracker settings file
 */
static const int NOF_SETTINGS = 3;


bool Settings::readSettings(const char *fname) {

	// create the xml reader instance
	TiXmlDocument doc(fname);
	if(!doc.LoadFile()) {
		return false;
	}

	// Is the root available?
	TiXmlElement *rootElement = doc.RootElement();
	if(!rootElement) {
		return false;
	}

	std::string *plist[] = {&oput_dir,
							&dev1,
							&dev2,
							&gazetrackerFile,
							&eyeCamCalibFile,
							&sceneCamCalibFile,
							&mapperFile};

	// attributes
	std::vector<std::string> target_attr(NOF_SETTINGS);
	target_attr[0] = std::string("output");
	target_attr[1] = std::string("input_devices");
	target_attr[2] = std::string("input_files");

	// 
	std::vector<std::vector<std::string> > parameters(NOF_SETTINGS);
	parameters[0].resize(1); // 1 output directory
	parameters[1].resize(2); // 2 input devices
	parameters[2].resize(4); // 4 settings files

	// 1 output direcotry
	parameters[0][0] = std::string("directory");

	// 2 devices
	parameters[1][0] = std::string("dev1");
	parameters[1][1] = std::string("dev2");

	// 3 input files
	parameters[2][0] = std::string("gazetracker");
	parameters[2][1] = std::string("eyeCamCalibration");
	parameters[2][2] = std::string("sceneCamCalibration");
	parameters[2][3] = std::string("mapper");


	std::string **ptr = plist;

	for(int i = 0; i < NOF_SETTINGS; ++i) {

		const std::vector<std::string> &cur_parameters = parameters[i];

		for(size_t j = 0; j < cur_parameters.size(); ++j) {

			**ptr = getString(rootElement, target_attr[i].c_str(), cur_parameters[j].c_str());

			if((*ptr)->empty() && **ptr != "") {

				printf("Could not get %s: %s\n", target_attr[i].c_str(), cur_parameters[j].c_str());

				return false;

			}

			++ptr;

		}

	}

	return true;

}


std::string Settings::getString(TiXmlElement *rootElement, const char *attr, const char *param) {

	std::string empty_str;

	// Is there any setting for the given file?
	TiXmlElement *elSettings = rootElement->FirstChildElement("settings");
	if(!elSettings) {
		return empty_str;
	}


	// loop through the elements and identify the attribute
	for( ; elSettings; elSettings = elSettings->NextSiblingElement() ) {
		const char* cur_attr = elSettings->Attribute("id");
		if(!cur_attr) {
			continue;
		}

		if(strcmp(cur_attr, attr) == 0) {
			break;
		}
	}

	if(!elSettings) {
		return empty_str;
	}

	// Is there a field for the parameter?
	TiXmlElement *parameterElement = elSettings->FirstChildElement(param);
	if(!parameterElement) {
		return empty_str;
	}

	// Does the field have a value?
	const char* valueData = parameterElement->Attribute("value");
	if(!valueData) {
		return empty_str;
	}

	return std::string(valueData);

}

