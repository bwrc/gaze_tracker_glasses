#include "JPEGWorker.h"
#include "jpeg.h"
#include <iostream>



JPEGWorker::JPEGWorker() : StreamWorker() {

}


JPEGWorker::~JPEGWorker() {

}


/*
 *
 */
CameraFrame *JPEGWorker::process(CameraFrame *img_compr) {

	int w = img_compr->w;
	int h = img_compr->h;
	int bpp = img_compr->bpp;


	// compressed data
	const unsigned char *data_compr = img_compr->data;


	// decompressed data, allocate here
	unsigned char *data_decompr = new unsigned char[bpp*w*h];


	// create the JPEG decompressor and decompress the frame, use the preallocated data
	JPEG_Decompressor jpgd;
	bool success = jpgd.decompress(data_compr, img_compr->sz, data_decompr);

	// if unseccesfull, make a white frame
	if(!success) {

		std::cout << "JPEGWorker::process(): Could not decompress, setting frame to white" << std::endl;

		// set the frame white
		memset(data_decompr, (unsigned char)255, bpp*w*h);

	}

	/*
	 * create a new frame with the decompressed data, do not copy
	 * the data, use the pointer instead
	 */
	CameraFrame *img_raw = new CameraFrame(w,
										   h,
										   bpp,
										   data_decompr,
										   w*h*bpp,
										   FORMAT_RGB,
										   false,		// do not copy, take the pointer instead
										   true);		// become parent of the data


	return img_raw;

}

