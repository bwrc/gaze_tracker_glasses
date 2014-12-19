#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <stdlib.h>

#include "Pattern.h"
#include "Pupil.h"


static const int RIGHT	= 0;
static const int LEFT	= 1;
static const int UP		= 2;
static const int DOWN	= 3;
static const int SPEED	= 6;


/******************************************************************************
 * Prototypes
 ******************************************************************************/
void drawPoints(cv::Mat &img_bgr, const Pattern &pattern);
void drawPupil(cv::Mat &img_bgr, const Pupil &pupil);
void movePupil(Pupil &pupil, int direction, const Pattern &pattern);
void handleKeyboard();
void analyse_and_draw(cv::Mat &img_bgr);


/******************************************************************************
 * Globals
 ******************************************************************************/

static const int WIN_W	= 640;
static const int WIN_H	= 480;

// pattern
Pattern pattern(cv::Size(WIN_W, WIN_H));

// pupil
Pupil pupil(100, WIN_W / 2, WIN_H / 2);

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
	while(b_running) {

		// clear the window
		cv::rectangle(img_bgr, cv::Point(0, 0), cv::Point(WIN_W, WIN_H), cv::Scalar(0, 0, 0), CV_FILLED);

		// draw the pupil
		drawPupil(img_bgr, pupil);

		// get the points from the pattern and draw them
		drawPoints(img_bgr, pattern);

		// analyse and draw the results
		analyse_and_draw(img_bgr);

		// show the image
		cv::imshow(win_name, img_bgr);

		handleKeyboard();

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
		cv::Scalar color = it->b_visible ? cv::Scalar(255, 255, 0) : cv::Scalar(50, 50, 50);

		const cv::Point &p = it->p;

		cv::circle(img_bgr, p, radius, color, solid, CV_AA);

		++it;
	}

}


void drawPupil(cv::Mat &img_bgr, const Pupil &pupil) {

	cv::Point p;
	pupil.getPos(p.x, p.y);

	int radius = pupil.getRadius();

	cv::circle(img_bgr, p, radius, cv::Scalar(255, 100, 255), CV_FILLED, CV_AA);

}


void analyse_and_draw(cv::Mat &img_bgr) {

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

	for(size_t i = 0; i < labels.size(); ++i) {

		char txt[10];
		sprintf(txt, "%d", labels[i]);

        //		cv::putText(img_bgr, std::string(txt), glints[labels[i]].p, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
        cv::putText(img_bgr, std::string(txt), points[i], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));

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


void handleKeyboard() {

	// get the key state
	const int key = cv::waitKey(2);

	if((char)key == 27) {
		b_running = false;
	}

	else if((char)key == 81) {	// left key
		movePupil(pupil, LEFT, pattern);
	}

	else if((char)key == 83) {	// right key
		movePupil(pupil, RIGHT, pattern);
	}

	else if((char)key == 82) {	// up key
		movePupil(pupil, UP, pattern);
	}

	else if((char)key == 84) {	// down key
		movePupil(pupil, DOWN, pattern);
	}

	else if((char)key >= '0' && (char)key <= '5') {

		size_t id = (size_t)((char)key - '0');

		pattern.toggle(id);
	}

}

