#include <iostream>

#include "settingsIO.h"
#include "trackerSettings.h"
#include "localTrackerSettings.h"
#include "PupilTracker.h"

// Global settings
TrackerSettings trackerSettings;

TrackerSettings::TrackerSettings()
{
	// Load the default values
	LocalTrackerSettings settings;
	this->set(settings);
}

#define getSettings(settingName)	{std::list<TrackerSetting>::iterator setting = settings.getSettingObject(#settingName); this->settingName = setting->getValue(); }

/******************************************************************************
 * This method loads the settings from LocalTrackerSettings-object
 *****************************************************************************/

void TrackerSettings::set(LocalTrackerSettings& settings)
{
	// Pupil tracker
	getSettings(MAX_NOF_CLUSTERS);
	getSettings(MAX_CLUSTER_SIZE);
	getSettings(MIN_CLUSTER_SIZE);
	getSettings(NOF_RAYS);
	getSettings(MAX_PUPIL_RADIUS);
	getSettings(MIN_NOF_RADIUSES);
	getSettings(MIN_PUPIL_RADIUS);
	getSettings(MIN_PUPIL_AREA);
	getSettings(NOF_VERIFICATION_FRAMES);
	getSettings(MAX_NOF_BAD_FRAMES);
	getSettings(ROI_W_DEFAULT);
	getSettings(ROI_H_DEFAULT);
	getSettings(ROI_MULTIPLIER);
	getSettings(MIN_ROI_W);
	getSettings(MAX_FAILED_TRACKS_BEFORE_RESET);
	getSettings(CR_THRESHOLD);
	getSettings(PUPIL_THRESHOLD);
	getSettings(AUTO_THRESHOLD);
	getSettings(CR_MAX_ERR_MULTIPLIER);
	getSettings(CROP_AREA_X);
	getSettings(CROP_AREA_Y);
	getSettings(CROP_AREA_W);
	getSettings(CROP_AREA_H);


	// Starburst
	getSettings(STARBURST_MAX_ITERATIONS);
	getSettings(STARBURST_REQUIRED_ACCURACY);
	getSettings(STARBURST_CIRCULAR_STEPS);
	getSettings(STARBURST_MIN_SEED_POINTS);
	getSettings(STARBURST_OUTLIER_DISTANCE);
	getSettings(STARBURST_RELATIVE_MAX_POINT_VARIANCE);
	getSettings(STARBURST_BLOCK_COUNT);
	getSettings(MAX_CR_WIDTH);


	// CRTemplate
	getSettings(CR_MASK_LEN);
	getSettings(rd);


	// GazeTracker
	getSettings(NOF_PERIMETER_POINTS);
	getSettings(MYY);

	// Cornea Computer
	getSettings(RHO);

    // gaze mapper
    getSettings(GAZE_DISTANCE);

}
