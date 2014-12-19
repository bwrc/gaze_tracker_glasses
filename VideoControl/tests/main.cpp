#include <cstdlib>
#include <iostream>

#include "VideoControl.h"

using namespace std;

int main(int argc, char* argv[])
{
	if(argc != 7) {
		cout << "Bad number of arguments" << endl;
		return 1;
	}

	string devName(argv[1]);
	VideoControl devControl(devName);

	if(!devControl.isOpen()) {
		cout << "Could not open the video device " << devName << endl;
	}

	if(!devControl.setExposure(EXPOSURE_MANUAL, strtod(argv[2], NULL) )) {
		cout << "Warning: Could not set exposure value" << endl;
	}
		
	if(!devControl.setFocus(false, strtod(argv[3], NULL) )) {
		cout << "Warning: Could not set focus value" << endl;
	}

	if(!devControl.setZoom(ZOOM_ABSOLUTE, strtod(argv[4], NULL) )) {
		cout << "Warning: Could not set zoom value" << endl;
	}

	if(!devControl.setTilt(strtod(argv[5], NULL) )) {
		cout << "Warning: Could not set tilt value" << endl;
	}

	if(!devControl.setPan(strtod(argv[6], NULL) )) {
		cout << "Warning: Could not set pan value" << endl;
	}

	devControl.close_dev();


	return 0;
}
