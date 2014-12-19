#include "group.h"
#include <stdio.h>


/****************************************************************************
 * Prototypes
 ****************************************************************************/
static void test(std::vector<cv::Point> &extracted);
static void test_with_six();
static void test_with_five();
static void test_with_four();
static void test_with_three();
static void test_with_two();


/****************************************************************************
 * Globals
 ****************************************************************************/
static std::vector<cv::Point> pattern_points(6);
static std::vector<cv::Point> pupil_centres(6);
static std::vector<std::string> headers(6);


int main() {

	static const int W = 200;
	static const int H = 200;

	/************************************************************************
	 * Create pattern points
	 ************************************************************************/

	pattern_points[0] = cv::Point(W/2,  0);
	pattern_points[1] = cv::Point(0,    H/2-H/8);
	pattern_points[2] = cv::Point(0,    H/2+H/8);
	pattern_points[3] = cv::Point(W/2,  H);
	pattern_points[4] = cv::Point(W,    H/2+H/8);
	pattern_points[5] = cv::Point(W,    H/2-H/8);



	/************************************************************************
	 * Create pupil centres
	 ************************************************************************/

	pupil_centres[0] = cv::Point(pattern_points[0].x - 5,	// upper left
								 pattern_points[0].y + 5);

	pupil_centres[1] = cv::Point(pattern_points[1].x + 5,	// middle left
								 pattern_points[1].y + 5);

	pupil_centres[2] = cv::Point(pattern_points[2].x + 5,	// bottom left
								 pattern_points[2].y + 5);

	pupil_centres[3] = cv::Point(pattern_points[3].x + 5,	// bottom right
								 pattern_points[3].y - 5);

	pupil_centres[4] = cv::Point(pattern_points[4].x - 5,	// middle right
								 pattern_points[4].y - 5);

	pupil_centres[5] = cv::Point(pattern_points[5].x - 5,	// upper right
								 pattern_points[5].y - 5);

	headers[0] = std::string("pupil centre: upper left");
	headers[1] = std::string("pupil centre: middle left");
	headers[2] = std::string("pupil centre: bottom left");
	headers[3] = std::string("pupil centre: bottom right");
	headers[4] = std::string("pupil centre: middle right");
	headers[5] = std::string("pupil centre: upper right");


	/************************************************************************
	 * The tests
	 ************************************************************************/

	printf("\nTesting with six\n");
	test_with_six();

	printf("\n\nTesting with five\n");
	test_with_five();

	printf("\n\nTesting with four\n");
	test_with_four();

	printf("\n\nTesting with three\n");
	test_with_three();

	printf("\n\nTesting with two\n");
	test_with_two();


	return EXIT_SUCCESS;
}


void test(std::vector<cv::Point> &extracted) {

	std::vector<cv::Point>::iterator it_pupil = pupil_centres.begin();
	std::vector<std::string>::iterator it_headers = headers.begin();

	while(it_pupil < pupil_centres.end()) {

		printf("\n************************************************\n");
		printf("%s\n\n", (it_headers++)->c_str());

		// create the group manager
		GroupManager grp(extracted, *it_pupil);

		// get the groups
		const std::vector<Group> &groups = grp.getGroups();

		// list the contents of the groups
		std::vector<Group>::const_iterator grp_it = groups.begin();

		while(grp_it < groups.end()) {

			// get the members of the group
			const std::vector<Element> &members = grp_it->getMembers();

			int grp_id = grp_it->getID();

			printf("Group %d: ", grp_id);


			std::vector<Element>::const_iterator member_it = members.begin();
			while(member_it < members.end()) {

				// list the labels
				printf("%d ", member_it->label);

				++member_it;
			}

			printf("\n");

			++grp_it;

		}

		printf("************************************************\n");

		++it_pupil;
	}

}


void test_with_six() {

	std::vector<cv::Point> extracted = pattern_points;

	test(extracted);

}


void test_with_five() {

	std::vector<cv::Point> extracted = pattern_points;

	std::vector<cv::Point>::iterator begin = extracted.begin();
	extracted.erase(begin + 3);

	test(extracted);
}


void test_with_four() {

	std::vector<cv::Point> extracted = pattern_points;

	std::vector<cv::Point>::iterator begin = extracted.begin();
	extracted.erase(begin + 3);
	extracted.erase(begin + 1);


	test(extracted);
}


void test_with_three() {

	std::vector<cv::Point> extracted = pattern_points;

	std::vector<cv::Point>::iterator begin = extracted.begin();
	extracted.erase(begin + 3);
	extracted.erase(begin + 2);
	extracted.erase(begin + 0);


	test(extracted);
}


void test_with_two() {

	std::vector<cv::Point> extracted = pattern_points;

	std::vector<cv::Point>::iterator begin = extracted.begin();
	extracted.erase(begin + 5);
	extracted.erase(begin + 3);
	extracted.erase(begin + 1);
	extracted.erase(begin + 0);

	test(extracted);
}

