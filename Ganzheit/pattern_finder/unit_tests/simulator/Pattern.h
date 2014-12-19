/*
 *	This class holds the information abouyt the pattern.
 *	It also renders the pattern on demand
 */

#ifndef PATTERN_H
#define PATTERN_H


#include <opencv2/imgproc/imgproc.hpp>


class Glint {
	public:
		Glint() {
			b_visible = true;
		}

		cv::Point p;
		bool b_visible;
	
};


class Pattern {
	public:
		Pattern(const cv::Size &img_dim);

		// get a reference to the points
		const std::vector<Glint> &getPoints() const {return points;}

		void identifyPoints(const cv::Point pupil_centre, std::vector<cv::Point> &extracted, std::vector<int> &labels);


		void toggle(size_t id);

	private:


		/*
         *
         *               o
         *
         *
         *
         *       o               o
         *
         *       o               o
         *
         *
         *
         *               o
         *
		 */

		std::vector<Glint> points;

};



#endif

