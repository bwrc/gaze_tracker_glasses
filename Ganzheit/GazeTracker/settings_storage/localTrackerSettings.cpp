#include <iostream>

#include "settingsIO.h"
#include "localTrackerSettings.h"
#include "PupilTracker.h"


void LocalTrackerSettings::addSetting(const char *file,
                                      const char *parameter,
                                      double min,
                                      double max,
                                      double defaultValue,
                                      double dInc)
{
	this->varList.push_back(TrackerSetting(std::string(file), std::string(parameter), min, max, defaultValue, dInc));
}

LocalTrackerSettings::LocalTrackerSettings(void)
{

	/******************************************************************************
	 * CRTemplate
	 *****************************************************************************/

	addSetting("CRTemplate",	"MAX_CR_WIDTH",			0,	17,	5, 1);         /* Maximum diameter of the CR cirlce. NOTE: Must be odd! */
	addSetting("CRTemplate",	"CR_MASK_LEN",			0,	51,	3 * 5, 1);     /* The length of the CR mask used in maskTest() */

	/******************************************************************************
	 * PupilTracker
	 *****************************************************************************/

	addSetting("PupilTracker",	"MAX_NOF_CLUSTERS",		0,	400,	200, 1);    /* maximum number of clusters */
	addSetting("PupilTracker",	"MAX_CLUSTER_SIZE",		0,	2000,	800, 1);    /* maximum size of a cluster */
	addSetting("PupilTracker",	"MIN_CLUSTER_SIZE",		0,	200,	30,  1);     /* minimum number of points in a cluster */
	addSetting("PupilTracker",	"NOF_RAYS",			    0,  200,    150, 1);        /* must be even */
	addSetting("PupilTracker",	"MAX_PUPIL_RADIUS",		0,	800,	400, 1);    /* maximum radius for a pupil */
	addSetting("PupilTracker",	"MIN_NOF_RADIUSES",		0,	40,	150 * 0.2, 1);  /* minimum number of radiuses used in ellipse fitting */
	addSetting("PupilTracker",	"MIN_PUPIL_RADIUS",		0,	40,	10, 1);         /* the minimum radius of a pupil */
	addSetting("PupilTracker",	"MIN_PUPIL_AREA",		0,	(int)(AREA(40) + 0.5),	(int)(AREA(10) + 0.5), 1);

	addSetting("PupilTracker",	"NOF_VERIFICATION_FRAMES",	0,	60,	5, 1);      /* Number of previous frames used in ellipse verification  */
	addSetting("PupilTracker",	"MAX_NOF_BAD_FRAMES",		0,	60,	2, 1);      /* Number of allowed contradictonary frames */

	addSetting("PupilTracker",	"ROI_W_DEFAULT",		0,	400,	250, 1);
	addSetting("PupilTracker",	"ROI_H_DEFAULT",		0,	400,	250, 1);
	addSetting("PupilTracker",	"MIN_ROI_W",			0,	400,	2 * 3 * 5, 1);
	addSetting("PupilTracker",	"MAX_FAILED_TRACKS_BEFORE_RESET",	1,	60,	10, 1);
	addSetting("PupilTracker",	"CR_THRESHOLD",			0,	255,	180, 1);
	addSetting("PupilTracker",	"PUPIL_THRESHOLD",			0,	255,	10, 1);
	addSetting("PupilTracker",	"AUTO_THRESHOLD",			0,	1,	1, 1);
	addSetting("PupilTracker",	"CR_MAX_ERR_MULTIPLIER",	0,	1,	0.3, 0.01);
	addSetting("PupilTracker",	"ROI_MULTIPLIER",		1,	5,	2, 0.01);

	addSetting("PupilTracker",	"CROP_AREA_X",		0,	1000,	0, 1);
	addSetting("PupilTracker",	"CROP_AREA_Y",		0,	1000,	0, 1);
	addSetting("PupilTracker",	"CROP_AREA_W",		0,	1000,	0, 1);
	addSetting("PupilTracker",	"CROP_AREA_H",		0,	1000,	0, 1);


	/******************************************************************************
	 * Starburst
	 *****************************************************************************/

	addSetting("Starburst",	"STARBURST_MAX_ITERATIONS",		0,	20,	10, 1);
	addSetting("Starburst",	"STARBURST_REQUIRED_ACCURACY",		0,	20,	10, 1);
	addSetting("Starburst",	"STARBURST_CIRCULAR_STEPS",		0,	180,	45, 1);
	addSetting("Starburst",	"STARBURST_MIN_SEED_POINTS",		0,	180,	30, 1);
	addSetting("Starburst",	"STARBURST_OUTLIER_DISTANCE",		0,	10,	1.5, 0.1);
	addSetting("Starburst",	"STARBURST_BLOCK_COUNT",		1,	5,	2, 1);
	addSetting("Starburst",	"STARBURST_RELATIVE_MAX_POINT_VARIANCE", 0,	3.0,	1.5, 0.1);


	/******************************************************************************
	 * Cornea Computer
	 *****************************************************************************/

	addSetting("CorneaComputer",	"RHO",				0.00370,	0.00970,	0.00770, 0.0001);


	/******************************************************************************
	 * GazeTracker
	 *****************************************************************************/

	/* The population average of the distance between the cornea centre and the pupil centre in millimeters */

	addSetting("GazeTracker",	"rd",				0.00145,	0.00600,	0.00680 - 0.00305, 0.0001);
	addSetting("GazeTracker",	"NOF_PERIMETER_POINTS",		30,	120,	60, 1);
	addSetting("GazeTracker",	"MYY",				0.1,	2.0,	1.0 / 1.0, 0.01);



	/******************************************************************************
	 * SceneMapper
	 *****************************************************************************/
	addSetting("SceneMapper",	"GAZE_DISTANCE",		0.1,	100.0,	1.0, 0.01);

}


std::list<TrackerSetting>::iterator LocalTrackerSettings::getFirstObject()
{
	if(this->varList.size() == 0) {
		return (std::list<TrackerSetting>::iterator)NULL;
	}

	return this->varList.begin();	
}

std::list<TrackerSetting>::iterator LocalTrackerSettings::getLastObject()
{
	if(this->varList.size() == 0) {
		return (std::list<TrackerSetting>::iterator)NULL;
	}

	return this->varList.end();	
}

std::list<TrackerSetting>::iterator LocalTrackerSettings::getSettingObject(const std::string& name)
{
	if(this->varList.size() == 0) {
		return (std::list<TrackerSetting>::iterator)NULL;
	}

	std::list<TrackerSetting>::iterator var = this->varList.begin();

	for(; var != this->varList.end(); var++) {
		if(name == var->getName()) {
			return var;
		}
	}

	return (std::list<TrackerSetting>::iterator)NULL;
}

std::list<TrackerSetting>::iterator LocalTrackerSettings::getSettingObject(const char * name)
{
	std::string settingName(name);
	return this->getSettingObject(settingName);
}

void LocalTrackerSettings::open(SettingsIO& settingsFile)
{
	std::list<TrackerSetting>::iterator var = this->varList.begin();

	for(; var != this->varList.end(); var++) {
		double value;

		if(settingsFile.get(var->getSubClass(), var->getName(), value)) {
			var->setValue(value);
		}
	}

}

void LocalTrackerSettings::save(SettingsIO& settingsFile)
{
	std::list<TrackerSetting>::iterator var = this->varList.begin();

	for(; var != this->varList.end(); var++) {
		settingsFile.set(var->getSubClass(), var->getName(), var->getValue());
	}

}

