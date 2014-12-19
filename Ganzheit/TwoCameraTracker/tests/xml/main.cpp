#include "CalibDataReader.h"


void printCamContainer(const calib::CameraCalibContainer &container);
void printLEDContainers(const std::vector<calib::LEDCalibContainer> &container);
void printLEDContainer(const calib::LEDCalibContainer &container);
void printLEDSample(const calib::LEDCalibSample &sample);


int main(int nargs, const char **args) {

	if(nargs != 2) {
		printf("Usage:\n  ./xmldump <filename>\n");
		return -1;
	}

	const char *fileName = args[1];

	CalibDataReader rder;

	if(!rder.create(fileName)) {

		printf("Could not create \"%s\"\n", fileName);
		return -1;

	}


	/******************************************************
	 * Extract camera information
	 ******************************************************/

	calib::CameraCalibContainer camContainer;
	if(!rder.readCameraContainer(camContainer)) {
		printf("Could not read the camera container\n");
		return -1;
	}

	// print the camera container
	printCamContainer(camContainer);



	/******************************************************
	 * Extract LED information
	 ******************************************************/
	std::vector<calib::LEDCalibContainer> LEDContainers;
	if(!rder.readLEDContainers(LEDContainers)) {
		printf("Could not read the LED container\n");
		return -1;
	}

	// print the camera container
	printLEDContainers(LEDContainers);

	return 0;

}


void printCamContainer(const calib::CameraCalibContainer &camContainer) {

	/*******************************************************
	 * Header
	 *******************************************************/
	printf(	"\n\n"
			"******************************************\n"
			"* Camera container\n"
			"******************************************\n"
			"\n");


	/*******************************************************
	 * Image size
	 *******************************************************/
	printf("  Image size:\n  %dx%d\n\n", camContainer.imgSize.width, camContainer.imgSize.height);


	/*******************************************************
	 * Intrinisic matrix
	 *******************************************************/
	printf(	"  Intrinsic matrix:\n"
			"    | %.2f %.2f %.2f |\n"
			"    | %.2f %.2f %.2f |\n"
			"    | %.2f %.2f %.2f |\n\n",
			camContainer.intr[0], camContainer.intr[3], camContainer.intr[6],
			camContainer.intr[1], camContainer.intr[4], camContainer.intr[7],
			camContainer.intr[2], camContainer.intr[5], camContainer.intr[8]);


	/*******************************************************
	 * Distortion coefficients
	 *******************************************************/
	const double *dist = camContainer.dist;
	printf(	"  Distortion coefficients:\n"
			"    %.2f %.2f %.2f %.2f %.2f\n\n",
			dist[0], dist[1], dist[2], dist[3], dist[4]);


	/*******************************************************
	 * Reprojection error
	 *******************************************************/
	printf("  Reprojection error:\n    %.2f\n\n", camContainer.reproj_err);



	/*******************************************************
	 * Object points
	 *******************************************************/
	const std::vector<cv::Point3f> &object_points = camContainer.object_points;

	printf("  Object points:\n");
	for(size_t i = 0; i < object_points.size(); ++i) {

		const cv::Point3f &p = object_points[i];

		printf("    (%.2f, %.2f, %.2f)\n", p.x, p.y, p.z);

	}
	printf("\n");



	const std::vector<calib::CameraCalibSample> &samples = camContainer.getSamples();
	printf("  ** %d samples\n", (int)samples.size());

	for(size_t i = 0; i < samples.size(); ++i) {

		const calib::CameraCalibSample &sample = samples[i];


		/*******************************************************
		 * Image points
		 *******************************************************/
		printf("    Image points:\n");
		for(size_t j = 0; j < sample.image_points.size(); ++j) {

			const cv::Point2f &p = sample.image_points[j];

			printf("      (%.2f, %.2f)\n", p.x, p.y);

		}
		printf("\n");

		/*******************************************************
		 * The path to the image
		 *******************************************************/
		printf("    Image path:\n      %s\n", sample.img_path.c_str());

	}

}


void printLEDContainers(const std::vector<calib::LEDCalibContainer> &containers) {

	/*******************************************************
	 * Header
	 *******************************************************/
	printf(	"\n\n"
			"******************************************\n"
			"* LED containers\n"
			"******************************************\n"
			"\n");

	size_t n = containers.size();

	for(size_t i = 0; i < n; ++i) {

		printLEDContainer(containers[i]);

	}

}


void printLEDContainer(const calib::LEDCalibContainer &container) {

	/*******************************************************
	 * Circle spacing
	 *******************************************************/
	printf("  Circle spacing:\n    %.2f\n\n", container.circleSpacing);


	/*******************************************************
	 * LED position
	 *******************************************************/
	printf("  LED position:\n  (%.2f, %.2f, %.2f)\n\n",
			container.LED_pos[0],
			container.LED_pos[1],
			container.LED_pos[2]
	);

	/*******************************************************
	 * Object points
	 *******************************************************/
	const std::vector<cv::Point3f> &object_points = container.object_points;

	printf("  Object points:\n");
	for(size_t i = 0; i < object_points.size(); ++i) {

		const cv::Point3f &p = object_points[i];

		printf("    (%.2f, %.2f, %.2f)\n", p.x, p.y, p.z);

	}
	printf("\n");

}


void printLEDSample(const calib::LEDCalibSample &sample) {

	/*******************************************************
	 * Image points
	 *******************************************************/
	printf("  Image points:\n");
	for(size_t i = 0; i < sample.image_points.size(); ++i) {

		const cv::Point2f &p = sample.image_points[i];

		printf("    (%.2f, %.2f) ", p.x, p.y);

	}
	printf("\n");


	/*******************************************************
	 * Glint
	 *******************************************************/

	printf("  Glint:\n    (%.2f, %.2f)\n", sample.glint.x, sample.glint.y);


	/*******************************************************
	 * Normal of the surface
	 *******************************************************/
	printf("  Normal:\n    (%.2f, %.2f, %.2f)\n", sample.normal.x, sample.normal.y, sample.normal.z);


	/*******************************************************
	 * Transformation matrix
	 *******************************************************/
	const cv::Mat &tn = sample.transformationMatrix;
	printf(	"  Transformation matrix:\n  "
			"    | %.2f %.2f %.2f %.2f |\n"
			"    | %.2f %.2f %.2f %.2f |\n"
			"    | %.2f %.2f %.2f %.2f |\n"
			"    | %.2f %.2f %.2f %.2f |\n",

	tn.at<float>(0, 0), tn.at<float>(0, 1), tn.at<float>(0, 2), tn.at<float>(0, 3),
	tn.at<float>(1, 0), tn.at<float>(1, 1), tn.at<float>(1, 2), tn.at<float>(1, 3),
	tn.at<float>(2, 0), tn.at<float>(2, 1), tn.at<float>(2, 2), tn.at<float>(2, 3),
	tn.at<float>(3, 0), tn.at<float>(3, 1), tn.at<float>(3, 2), tn.at<float>(3, 3));


	/*******************************************************
	 * The path to the image
	 *******************************************************/
	printf("  Image path:\n    %s", sample.img_path.c_str());

}

