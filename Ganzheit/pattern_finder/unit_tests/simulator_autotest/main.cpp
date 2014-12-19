#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Pattern.h"
#include "Pupil.h"


static const int RIGHT	= 0;
static const int LEFT	= 1;
static const int UP		= 2;
static const int DOWN	= 3;
static const int SPEED	= 6;


const int pupilPos[6][2] = {{230, 162}, {230, 240}, {230, 318}, {410, 162}, {410, 240}, {410, 318}};

/******************************************************************************
 * Prototypes
 ******************************************************************************/
void drawPoints(cv::Mat &img_bgr, const Pattern &pattern);
void drawPupil(cv::Mat &img_bgr, const Pupil &pupil);
void movePupil(Pupil &pupil, int direction, const Pattern &pattern);
void analyse_and_validate(cv::Mat &img_bgr, int pupilPosition);


/******************************************************************************
 * Globals
 ******************************************************************************/

static const int WIN_W	= 640;
static const int WIN_H	= 480;


int pat = 0;

// pattern
Pattern pattern(cv::Size(WIN_W, WIN_H));

// pupil
Pupil pupil(50, WIN_W / 2, WIN_H / 2);

// main loop state
bool b_running = true;

int main(int argc, char **argv) {

	/* initialize random seed: */
	srand(time(NULL));


	std::string win_name("Main");

	/**********************************************************************
	 * Initialize the main window
	 *********************************************************************/
	cv::namedWindow(win_name, CV_WINDOW_AUTOSIZE);

	// displayed image
	cv::Mat img_bgr(WIN_H, WIN_W, CV_8UC3);

	/**********************************************************************
	 * Main loop
	 *********************************************************************/

	for(pat = 0; pat != 0b00111111; pat++) {

		for(int i = 0; i < 6; i++)  {
			if((pat >> i) & 1) {
				pattern.set(i);
			} else {
				pattern.clear(i);
			}
		}

		for(int pos = 0; pos < 6; pos++) {

			pupil.moveTo(pupilPos[pos][0], pupilPos[pos][1]);

			// clear the window
			cv::rectangle(img_bgr, cv::Point(0, 0), cv::Point(WIN_W, WIN_H), cv::Scalar(255, 255, 255), CV_FILLED);

			// draw the pupil
			drawPupil(img_bgr, pupil);

			// get the points from the pattern and draw them
			drawPoints(img_bgr, pattern);

			// analyse and draw the results
			analyse_and_validate(img_bgr, pos);

		}

	}


	return EXIT_SUCCESS;
}


void drawPoints(cv::Mat &img_bgr, const Pattern &pattern) {

	const std::vector<Glint> &points = pattern.getPoints();

	std::vector<Glint>::const_iterator it = points.begin();

	// circle radius
	const int radius = 15;

	while(it != points.end()) {

		int solid = it->b_visible ? CV_FILLED : 2;
		cv::Scalar color = it->b_visible ? cv::Scalar(0, 255, 0) : cv::Scalar(50, 50, 50);

		const cv::Point &p = it->p;

		cv::circle(img_bgr, p, radius, color, solid, CV_AA);

		++it;
	}

}


void drawPupil(cv::Mat &img_bgr, const Pupil &pupil) {

	cv::Point p;
	pupil.getPos(p.x, p.y);

	int radius = pupil.getRadius();

	cv::circle(img_bgr, p, radius, cv::Scalar(0, 0, 0), CV_FILLED, CV_AA);

}


void analyse_and_validate(cv::Mat &img_bgr, int pupilPosition) {

	std::vector<int> labels;

	cv::Point pupil_centre;
	pupil.getPos(pupil_centre.x, pupil_centre.y);


	const std::vector<Glint> &glints = pattern.getPoints();
	std::vector<cv::Point> tmp_points;
	for(size_t i = 0; i < glints.size(); ++i) {

		if(glints[i].b_visible) {
			tmp_points.push_back(glints[i].p);
		}
	}

	std::vector<cv::Point> points(tmp_points.size());

	for(size_t i = 0; i < points.size(); ++i) {

		// 0 - 1
		double c = (double)rand() / (double)RAND_MAX;

		int rand_ind = (int)(c * (tmp_points.size() - 1) + 0.5);

		points[i] = tmp_points[rand_ind];

		tmp_points.erase(tmp_points.begin() + rand_ind);
	}


	pattern.identifyPoints(pupil_centre, points, labels);

	int foundStuff = 0;

	std::vector<Glint> pointsg = pattern.getPoints();

	for(size_t i = 0; i < labels.size(); ++i) {

		foundStuff |= 1 << labels[i];

		cv::Scalar color = cv::Scalar(255, 0, 0);
		cv::Point p = pointsg[labels[i]].p;
		cv::circle(img_bgr, p, 15, color, CV_FILLED, CV_AA);

	}

	if (foundStuff != pat && labels.size() > 1) {
		
		std::stringstream tmpstream;
		tmpstream << "Pattern_" << pat << "_" << foundStuff << ".jpg";
		std::string tmp = tmpstream.str();
		cv::imwrite(tmp, img_bgr);
		std::cout << "Pattern " << pat << " was recognised as " << foundStuff << ", glints seen: " << labels.size() << ", Pupil position: " << pupilPosition << std::endl;
	}
}


void movePupil(Pupil &pupil, int direction, const Pattern &pattern) {

	int x, y;
	pupil.getPos(x, y);

	switch(direction) {

		case LEFT: {

			x -= SPEED;

			break;
		}

		case RIGHT: {

			x += SPEED;

			break;
		}

		case UP: {

			y -= SPEED;

			break;
		}

		case DOWN: {

			y += SPEED;

			break;
		}

		default: break;

	}

	const std::vector<Glint> &glints = pattern.getPoints();
	std::vector<cv::Point> points(glints.size());
	for(size_t i = 0; i < points.size(); ++i) {
		points[i] = glints[i].p;
	}

	if(cv::pointPolygonTest(points, cv::Point2f(x, y), false) > 0) {
		pupil.moveTo(x, y);
	}

}

