GazeTracking class hierarchies

PupilTracker dependencies

Depends on:
Starburst
Clusteriser
CRTemplate
Iris



Functions that call methods of these classes/namespaces/files

bool PupilTracker::track()
    - Clusteriser::clusterise()


bool PupilTracker::define_ROI()
    - Starburst::process()


void PupilTracker::getCrCandidates()
    - CRTemplate::makeTemplateImage()
    - ellipse::pointInsideEllipse()
    - CRTemplate::maskTests()


cv::RotatedRect PupilTracker::trackEyeLids()
    - iris::burst()



GazeTracker dependencies

Depends on:
PupilTracker
Cornea_computer
Camera
group
ellipse
trackerSettings


bool GazeTracker::track()
    - PupilTracker::track()
    - SixLEDs::getCorneaCentre() --> Cornea_computer::computeCentre()
    - Camera::pixToWorld()


double GazeTracker::getRP()
    - Camera::pixToWorld()
    - ellipse::getEllipsePoints()


bool GazeTracker::SixLEDs::associateGlints()
    - group::GroupManager::identify()
    - group::GroupManager::getBestConfiguration()


voi GazeTracker::getEllipsePoints()
    - ellipse::getEllipsePoint()

