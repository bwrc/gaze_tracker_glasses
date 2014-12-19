#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <string.h>
#include <math.h>
#include "MyTimer.h"
#include "jpeg.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


class ImageRGB {

	public:
		ImageRGB() {
			w = 0;
			h = 0;
			data = NULL;
		}

		ImageRGB(int _w, int _h) {
			w = _w;
			h = _h;
			data = new unsigned char[3*w*h];
		}

		~ImageRGB() {
			delete[] data;
		}

		int w, h;
		unsigned char *data;
};




void draw(ImageRGB &image) {

	int w = image.w;
	int h = image.h;

	memset(image.data, 120, 3*w*h);

	int cx = w / 2;
	int cy = h / 2;
	int r = std::min(w, h) / 4;

	const int N = 360;

	const double pi = 4.0*atan(1.0);
	double ang = 0;
	double angd = 2.0*pi / (N-1.0);

	unsigned char *data = image.data;

	for(int i = 0; i < N; ++i) {

		int x = cx + r*cos(ang) + 0.5;
		int y = cy + r*sin(ang) + 0.5;

		int ind		= 3*(y*w + x);
		data[ind+0] = 255;
		data[ind+1] = 255;
		data[ind+2] = 255;

		ang += angd;

	}

}


int main() {

	// create an RGB image, and draw to it
	ImageRGB image(640, 480);
	draw(image);


	// create the timer
	MyTimer timer;


	/*****************************************************************
	 * Compress
	 *****************************************************************/
	JPEG_Compressor jpegc;
	jpegc.compress(image.data, image.w, image.h, 3);

	// print the duration
	unsigned long micros = timer.getElapsed_micros();
	unsigned long millis = micros / 1000.0 + 0.5;
	printf("*****************************************************************\n"
		   " Compress\n"
		   "*****************************************************************\n");
	printf("duration: %ld us\n", micros);
	printf("duration: %ld ms\n", millis);


	/* write the buffer to disk so you can see the image */
	FILE *outfile  = fopen("test.jpeg", "wb");
	size_t sz;
	const JOCTET *compr_data = jpegc.getCompressedData(sz);
	fwrite(compr_data, sizeof(JOCTET), sz, outfile);
	fclose(outfile);

	printf("compressed data size: %d\n", (int)sz);



	/*****************************************************************
	 * Decompress
	 *****************************************************************/
	timer.markTime();
	JPEG_Decompressor jpegd;
	jpegd.decompress(compr_data, sz);
	// print the duration
	micros = timer.getElapsed_micros();
	millis = micros / 1000.0 + 0.5;

	printf("\n\n*****************************************************************\n"
		   " Decompress\n"
		   "*****************************************************************\n");
	printf("duration: %ld us\n", micros);
	printf("duration: %ld ms\n", millis);




//cv::Mat i(480, 640, CV_8UC3);

size_t s;
const unsigned char *raw_data = jpegd.getRawData(s);

cv::Mat img(480, 640, CV_8UC3);
memcpy(img.data, raw_data, 3*640*480);
cv::imwrite(std::string("test.bmp"), img);


	return EXIT_SUCCESS;
}

