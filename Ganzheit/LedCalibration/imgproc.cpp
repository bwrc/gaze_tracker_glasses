#include "imgproc.h"
#include <stdio.h>



namespace imgproc {


class AVGBuffer {
	public:
		AVGBuffer(int _buff_sz) {
			count = 0;
			buff_sz = _buff_sz;
			buff = new int[buff_sz];
			init(NULL);
		}

		~AVGBuffer() {
			delete[] buff;
		}
		

		void initc(const unsigned char *b) {
			ind = 0;
			sum = 0;

			for(int i = 0; i < buff_sz; ++i) {
				int val = (int)b[i];
				buff[i] = val;
				sum += val;
			}
			
		}

		void init(const int *b) {
			ind = 0;
			sum = 0;

			if(b != NULL) {
				memcpy(buff, b, buff_sz*sizeof(int));
				for(int i = 0; i < buff_sz; ++i) {
					sum += b[i];
				}
			}
			else {
				memset(buff, 0, buff_sz*sizeof(int));
			}
		}

		void add(int _sample) {
			sum -= buff[ind];
			buff[ind] = _sample;
			ind = (ind + 1) % buff_sz;

			sum += _sample;
			count = std::min(count+1, buff_sz);
		}

		int avg() {
			return (int)(sum / (double)count + 0.5);
		}

	private:
		int buff_sz;
		int *buff;
		int ind;
		int sum;
		int count;
};



bool default_comparator(unsigned char val, unsigned char th) {
	return val >= th;
}


void findFromLine(const cv::Mat &img_gray,
				  int x0,
				  int y0,
				  int x1,
				  int y1,
				  unsigned char threshold,
				  cv::Point &ret_next,
				  cv::Point &ret_prev,
				  comparator comp) {


	if(comp == NULL) {
		comp = default_comparator;
	}

	/*------ http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm ------

	  -----------------------------------------------------------------------		

		This is an optimised integer based line drawing method. Instead of

		drawing the line we go along it and detect a bright pixel.
	  -----------------------------------------------------------------------*/
	bool steep = abs(y1 - y0) > abs(x1 - x0);

	ret_next.x	= -1;
	ret_next.y	= -1;
	ret_prev.x	= -1;
	ret_prev.y	= -1;

	if(steep) {
		std::swap(x0, y0);
		std::swap(x1, y1);
	}

	int deltax = abs(x1 - x0);
	int deltay = abs(y1 - y0);
	int error = deltax / 2;
	int x = x0;
	int y = y0;
	int ystep = y0 < y1 ? 1 : -1;

	int inc;
	if(x0 < x1)	{inc =  1;}
	else		{inc = -1;}

	int *px = &x;
	int *py = &y;

	if(steep) {
		px = &y;
		py = &x;
	}


	const unsigned char * const pixels = img_gray.data;
	const int step = img_gray.step;

	while(x != x1) {

		if(comp(pixels[step * (*py) + (*px)], threshold)) {	// val, th
			ret_next.x = *px;
			ret_next.y = *py;
			break;
		}

ret_prev.x = *px;
ret_prev.y = *py;

		error -= deltay;
		if(error < 0) {
			y += ystep;
			error += deltax;
		}

		x += inc;
	}

}


void setEndPoints(const int x1,
				  const int y1,
				  int &x2,
				  int &y2,
				  const int distX,
				  const int distY,
				  const int img_w,
				  const int img_h) {


	// end points of the ray. Give a value for x2 and y2
	x2 = x1 + distX;
	y2 = y1 + distY;

	/* If x2 is not within the acceptable bounds, restrict its value. set y2
	   according to x2. */
	if(x2 < 0) {
		int x2_old = x2;
		x2 = 0;

		/* solve for y2, remember that x2 = 0:
   
		   (y2 - y1)		(y2' - y1')
		   ---------	=	-----------
		   (x2 - x1)		(x2' - x1')
	   */

		y2 = (int)((y2 - y1) * (-x1) / (double)(x2_old - x1) + y1 + 0.5);
	}
	else if(x2 >= img_w) {
		int x2_old = x2;
		x2 = img_w - 1;
		y2 = (int)((y2 - y1) * (x2 - x1) / (double)(x2_old - x1) + y1 + 0.5);
	}

	/* If y2 is not within the acceptable bounds, restrict its value and set x2
	   accordingly. Above x2 was already set and possibly restricted. Will setting it
	   according to y2 possibly put it out of the acceptable range?. The answer is no,
	   because here x2 is adjusted in such a way that it gets closer to x1. */
	if(y2 < 0) {
		int y2_old = y2;
		y2 = 0;
		x2 = (int)((x2 - x1) * (-y1) / (double)(y2_old - y1) + x1 + 0.5);
	}
	else if(y2 >= img_h) {
		int y2_old = y2;
		y2 = img_h - 1;
		x2 = (int)((x2 - x1) * (y2 - y1) / (double)(y2_old - y1) + x1 + 0.5);
	}
}



// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0) {
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


void removeOutliers(std::vector<cv::Point> &edges, double var_coeff) {

	double meanx	= 0;
	double meany	= 0;
	double squarex	= 0;
	double squarey	= 0;


	// compute the mean and square
	for(std::vector<cv::Point>::iterator it = edges.begin(); it != edges.end(); ++it) {

		meanx += it->x;
		meany += it->y;

		squarex += it->x * it->x;
		squarey += it->y * it->y;

	}

	size_t n = edges.size();

	meanx	/= n;
	meany	/= n;
	squarex	/= n;
	squarey	/= n;


	// compute the variance
	double varx = meanx * meanx - squarex;
	double vary = meany * meany - squarey;

	double var = std::sqrt(varx * varx + vary * vary);


	// Remove outliers.
	std::vector<cv::Point>::iterator it;

	for(it = edges.begin(); it != edges.end(); ++it) {

		double diff_x = it->x - meanx;
		double diff_y = it->y - meany;

		// see if the distance is greater than 1.5 times the variance
		if(sqrt(diff_x * diff_x + diff_y * diff_y) > (var_coeff * sqrt(var))) {

			// Take the element to remove
			std::vector<cv::Point>::iterator remove_pos = it;

			// Is this the first element?
			if(remove_pos != edges.begin()) {
				// No.. just remove it
				it--;
				edges.erase(remove_pos);
			} else {
				// We're removing the first element... remove and reset iterator.
				edges.erase(remove_pos);
				it = edges.begin();
				continue;
			}
		}

	}
}


void removeOutliers(std::vector<double> &arr) {

	double mean		= 0;
	double square	= 0;


	// compute the mean and square
	for(std::vector<double>::iterator it = arr.begin(); it != arr.end(); ++it) {

		mean += *it;
		square += (*it) * (*it);

	}

	size_t n = arr.size();

	mean	/= n;
	square	/= n;

	// compute the variance
	double var = square - mean * mean;

	// Remove outliers.
	std::vector<double>::iterator it;

	for(it = arr.begin(); it != arr.end(); ++it) {

		double diff = *it - mean;

		// see if the distance is greater than 1.5 times the variance
		if(std::abs(diff) > 1.2 * sqrt(var)) {

			// Take the element to remove
			std::vector<double>::iterator remove_pos = it;

			// Is this the first element?
			if(remove_pos != arr.begin()) {
				// No.. just remove it
				it--;
				arr.erase(remove_pos);
			} else {
				// We're removing the first element... remove and reset iterator.
				arr.erase(remove_pos);
				it = arr.begin();
				continue;
			}
		}

	}
}


} // end of "namespace imgproc"

