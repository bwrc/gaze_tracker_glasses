#include "Pattern.h"
#include <stdio.h>
#include "group.h"



using namespace group;

static const int N_POINTS = 6;


Pattern::Pattern(const cv::Size &img_dim) {

	// populate the point list
	points.resize(N_POINTS);

	/*
	 * WIDTH	= 4
	 * HEIGHT	= 3
	 *
     *                0
     *                o
     *
     *      1 o             o 5
     *
     *      2 o             o 4
     *
     *                o
     *                3
	 */

	// min dim
	int min_dim = std::min(img_dim.width, img_dim.height);

	// width and height, i.e. the bounding rectangle, of the pattern in pixels
	int pattern_w = 0.7 * min_dim;
	int pattern_h = 0.7 * min_dim;

	// centre of the image
	int half_x = (int)(img_dim.width / 2.0);
	int half_y = (int)(img_dim.height / 2.0);


	std::vector<cv::Point2f> pointsf(6);

	// 0
	pointsf[0].x = 0;
	pointsf[0].y = -1.5;

	// 1
	pointsf[1].x = -2;
	pointsf[1].y = -0.5;

	// 2
	pointsf[2].x = -2;
	pointsf[2].y =  0.5;

	// 3
	pointsf[3].x = 0;
	pointsf[3].y = 1.5;

	// 4
	pointsf[4].x = 2;
	pointsf[4].y = 0.5;

	// 5
	pointsf[5].x = 2;
	pointsf[5].y = -0.5;



	for(size_t i = 0; i < points.size(); ++i) {
		Glint &g		= points[i];
		cv::Point2f &p	= pointsf[i];

		g.p.x = half_x + pattern_w * (p.x / 4.0);
		g.p.y = half_y + pattern_h * (p.y / 4.0);
	}

}


void Pattern::toggle(size_t id) {

	if(id >= 0 && id < points.size()) {

		bool &state = points[id].b_visible;

		state = !state;
	}

}





void s(std::vector<Group> &groups, std::vector<Group>::iterator &git);
int vacancy[6];


void Pattern::identifyPoints(const cv::Point pupil_centre, std::vector<cv::Point> &extracted, std::vector<int> &labels) {

	// resize labels if necessary
	if(labels.size() != extracted.size()) {
		labels.resize(extracted.size());
	}


//printf("**********************************\n");
//	std::vector<Group>::iterator git = groups.begin();
//	s(groups, git);
//printf("**********************************\n\n");



//////printf("**********************************\n");
//////	GroupManager grp(extracted, pupil_centre);
//////	std::vector<Group> &groups = grp.getGroups();

//////	for(int i = 0; i < 6; ++i) {vacancy[i] = 1;}
//////	std::vector<Group>::iterator git = groups.begin();
//////	s(groups, git);
//////printf("**********************************\n\n");


	GroupManager grp;
	if(grp.identify(extracted, pupil_centre)) {

		const Configuration &configurations = grp.getBestConfiguration();

        //		std::list<int>::const_iterator lit = configurations.labels.begin();
        std::vector<Element>::const_iterator elit = configurations.elements.begin();

		for(size_t i = 0; i < labels.size(); ++i) {

			labels[i] = elit->label;

			++elit;
		}

	}

}

