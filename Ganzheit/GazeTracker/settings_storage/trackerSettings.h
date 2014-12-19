#ifndef TRACKERSETTINGS_H
#define TRACKERSETTINGS_H

#include <iostream>
#include <list>

#include "settingsIO.h"
#include "localTrackerSettings.h"

class TrackerSettings {
public:
    TrackerSettings(void);

    void set(LocalTrackerSettings& settings);

    // Pupil tracker
    int MAX_NOF_CLUSTERS;
    int MAX_CLUSTER_SIZE;
    int MIN_CLUSTER_SIZE;
    int MAX_PUPIL_RADIUS;
    int MIN_PUPIL_RADIUS;
    int NOF_RAYS;
    int MIN_NOF_RADIUSES;

    int MIN_PUPIL_AREA;
    unsigned int NOF_VERIFICATION_FRAMES;
    unsigned int MAX_NOF_BAD_FRAMES;
    int ROI_W_DEFAULT;
    int ROI_H_DEFAULT;
    double ROI_MULTIPLIER;
    int MIN_ROI_W;
    int MAX_FAILED_TRACKS_BEFORE_RESET;
    int CR_THRESHOLD;
    int PUPIL_THRESHOLD;
    int AUTO_THRESHOLD; // 0 or 1
    double CR_MAX_ERR_MULTIPLIER;
    int CROP_AREA_X;
    int CROP_AREA_Y;
    int CROP_AREA_W;
    int CROP_AREA_H;


    // Starburst
    int STARBURST_MAX_ITERATIONS;
    int STARBURST_REQUIRED_ACCURACY;
    int STARBURST_CIRCULAR_STEPS;
    int STARBURST_MIN_SEED_POINTS;
    int STARBURST_BLOCK_COUNT;
    double STARBURST_OUTLIER_DISTANCE;
    double STARBURST_RELATIVE_MAX_POINT_VARIANCE;

    // CRTemplate
    int MAX_CR_WIDTH;
    int CR_MASK_LEN;

    // GazeTracker
    double rd;
    int NOF_PERIMETER_POINTS;
    double MYY;

    // Cornea Computer
    double RHO;

    // gaze mapper
    double GAZE_DISTANCE;

};

extern TrackerSettings trackerSettings;

#endif
