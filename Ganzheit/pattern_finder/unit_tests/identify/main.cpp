#include "group.h"
#include <stdio.h>
#include <fstream>


using namespace group;


bool readConfig(const std::string &file, std::vector<cv::Point> &p);
void remove_whitespace(char *txt);
bool getCoords(char *txt, int *x, int *y);
void draw(const std::vector<cv::Point> &points,const cv::Point &pupil_centre);
bool mysort(cv::Point i, cv::Point j) {return (i.y < j.y);}



int main(int nargs, char **args) {

	if(nargs != 2) {
		printf("Give input file\n");
		return 0;
	}


	/************************************************************************
	 * Read the pattern points
	 ************************************************************************/
	const char *fIn = args[1];
	std::vector<cv::Point> extracted;
	bool ret = readConfig(std::string(fIn), extracted);

	if(!ret) {
		printf("Could not read input file '%s'\n", fIn);
		return 0;
	}


	/******************************************
	 * Print the read coordinates
	 ******************************************/
	printf("*****************************************\n");
	printf("* Reading points\n");
	printf("*****************************************\n");
	for(size_t i = 0; i < extracted.size(); ++i) {

		const int x = extracted[i].x;
		const int y = extracted[i].y;

		printf("%d, %d\n", x, y);
	}
	printf("\n");


	/************************************************************************
	 * Create the pupil centre
	 ************************************************************************/
	cv::Point pupil_centre(400, 300);


	/************************************************************************
	 * Draw the scenario in ASCII graphics
	 ************************************************************************/
	draw(extracted, pupil_centre);


	GroupManager grp;
	bool success = grp.identify(extracted, pupil_centre);

	if(success) {
		const std::vector<Configuration> &configs = grp.getConfigurations();

		for(size_t i = 0; i < configs.size(); ++i) {

			const std::vector<group::Element> &elements = configs[i].elements;
			std::vector<group::Element>::const_iterator elit = elements.begin();
			std::vector<group::Element>::const_iterator end = elements.end();

			printf("\nConfiguration %d, err = %.2f\n", (int)(i + 1), configs[i].err);

			while(elit != end) {
				printf("  %d", elit->label);
				++elit;
			}

		}

		printf("\n\nAnd the best configuration is:\n");

		const Configuration &config_best = grp.getBestConfiguration();

		std::vector<group::Element>::const_iterator elit = config_best.elements.begin();

		for(; elit != config_best.elements.end(); ++elit) {
			printf("%d  ", elit->label);
		}
		printf("\n\n");

	}
	else {
		printf("identification failed\n");
	}


	return EXIT_SUCCESS;
}


bool readConfig(const std::string &file, std::vector<cv::Point> &pattern_points) {

	std::ifstream in(file.c_str(), std::ifstream::in);

	if(!in.is_open()) {
		return false;
	}

	char line[256];

	while(!in.eof()) {

		in.getline(line, 256);
		remove_whitespace(line);

		if(strlen(line) == 0) {
			continue;
		}

		int x, y;
		if(!getCoords(line, &x, &y)) {
			return false;
		}

		pattern_points.push_back(cv::Point(x, y));

	}


	in.close();

	return true;

}


void remove_whitespace(char *txt) {

	const int len = strlen(txt);
	char *tmp = new char[len + 1];
	memset(tmp, '\0', len + 1);

	char *ptmp = &tmp[0];

	for(int i = 0; i < len; ++i) {

		const char c = txt[i];

		if(c != ' ' && c != '\t') {
			*ptmp = c;
			++ptmp;
		}

	}

	strcpy(txt, tmp);

	delete[] tmp;

}


bool getCoords(char *txt, int *x, int *y) {

	char *pch = strchr(txt, ',');

	if(pch == NULL) {
		return false;
	}

	*x = atoi(txt);
	*y = atoi(pch + 1);

	return true;

}


void draw(const std::vector<cv::Point> &points,const cv::Point &pupil_centre) {

	const size_t sz = points.size();

	// get max and min for x and y
	int max_x = -1;
	int max_y = -1;
	int min_x = INT_MAX;
	int min_y = INT_MAX;

	for(size_t i = 0; i < sz; ++i) {

		const cv::Point &p = points[i];

		int x = p.x;
		int y = p.y;

		// for max
		if(x > max_x) {
			max_x = x;
		}

		if(y > max_y) {
			max_y = y;
		}

		// for min
		if(x < min_x) {
			min_x = x;
		}

		if(y < min_y) {
			min_y = y;
		}

	}

	const int w = max_x - min_x + 1;
	const int h = max_y - min_y + 1;


	// max rows and cols when printing on the console
	const int max_w = 60;
	const int max_h = 30;
	std::vector<cv::Point> pixels(sz);

	for(size_t i = 0; i < sz; ++i) {

		const cv::Point &p = points[i];

		const int x = p.x;
		const int y = p.y;

		const double ratio_x = (double)(x - min_x) / (double)w;
		const double ratio_y = (double)(y - min_y) / (double)h;

		const int nx = (int)(ratio_x * max_w + 0.5);
		const int ny = (int)(ratio_y * max_h + 0.5);

		pixels[i].x = nx;
		pixels[i].y = ny;

	}

	// sort ascending according to y
	std::sort(pixels.begin(), pixels.end(), mysort);


	// draw

	printf("*****************************************\n");
	printf("* Pattern\n");
	printf("*****************************************\n\n");

	for(size_t i = 0; i < sz; ++i) {

		int stepy = i == 0 ? 0 : pixels[i].y - pixels[i-1].y;

		for(int ny = 0; ny < stepy; ++ny) {
			printf("\n");
		}

		int stepx = pixels[i].x;
		for(int nx = 0; nx < stepx-1; ++nx) {
			printf(" ");
		}
		printf("o");

	}

	printf("\n\n");

}

