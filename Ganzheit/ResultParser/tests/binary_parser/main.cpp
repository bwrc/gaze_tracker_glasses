#include "BinaryResultParser.h"
#include <stdio.h>
#include <sys/time.h>
#include <vector>


static const uint32_t MAX_UINT32 = 4294967295;


void binary_from_data(const ResultData &data, std::vector<char> &buff);
void printData(const ResultData &data);


bool bIsLittleEndian = false;

bool isLittleEndian() {

	int a = 1;
	char *ca = (char *)&a;

	return ca[0] == 1;

}



int main(int nof_args, const char **list_args) {

	bIsLittleEndian = isLittleEndian();

	/***************************************************
	 * Create original data
	 ***************************************************/
	ResultData orig_data;
	orig_data.id = 3;
	orig_data.timestamp = 1337939183;
	orig_data.track_dur_micros = 4894;
	orig_data.pupil = cv::RotatedRect(cv::Point2f(446.52896, 196.62518),
									  cv::Size2f(14.42952, 31.37766),
									  261.15677);

	orig_data.cornea_centre	= cv::Point3d(5.02840, -0.95307, 18.058399);
	orig_data.pupil_centre	= cv::Point3d(3.62954, -1.08628, 15.745886);
	orig_data.scenePoint	= cv::Point2d(99.0, 679.9);
	orig_data.glints.push_back(cv::Point2d(381.00000, 238.00000));
	orig_data.glints.push_back(cv::Point2d(392.00000,146.00000));
	orig_data.glints.push_back(cv::Point2d(406.00000,258.00000));
	orig_data.glints.push_back(cv::Point2d(469.00000, 258.00000));
	orig_data.glints.push_back(cv::Point2d(510.00000, 141.00000));


	/***************************************************
	 * Generate the buffer
	 ***************************************************/
	std::vector<char> buff;
	binary_from_data(orig_data, buff);


	/***************************************************
	 * Parse the buffer into data
	 ***************************************************/
	struct timeval t1, t2;
	gettimeofday(&t1, 0);
	ResultData data;

	if(!BinaryResultParser::parsePacket(buff.data(), buff.size(), data)) {

		printf("*****************************************\n"
			   "* !!! Could not parse the packet !!!\n"
			   "*****************************************\n");

		return -1;

	}
	else {

		gettimeofday(&t2, 0);

		long diff_sec			= t2.tv_sec - t1.tv_sec;
		long diff_micros		= t2.tv_usec - t1.tv_usec;
		unsigned long micros	= 1000000 * diff_sec + diff_micros;

		printf("*****************************************\n"
			   "* Parsed in:\n"
			   "*****************************************\n");

		printf("micros: %ld us\n", micros);
		printf("millis: %.2f ms\n", micros / 1000.0);

	}

	printf("\n\n*****************************************\n"
		   "* Parsed data\n"
		   "*****************************************\n");

	printData(data);
	printf("\n");

	return 0;

}


/* Unsigned 4-byte integer to litte-endian 4-byte buffer */
void UINT32_TO_4BYTE_LE(uint32_t val, char *buff) {

	buff[0] = (val & 0x000000FF);
	buff[1] = (val & 0x0000FF00) >> 8;
	buff[2] = (val & 0x00FF0000) >> 16;
	buff[3] = (val & 0xFF000000) >> 24;

}


/*
 * 4-byte floating point number to litte-endian 4-byte buffer
 */
void FLOAT_TO_4BYTE_LE(float val, char *buff) {

	char *cval = (char *)&val;

	if(bIsLittleEndian) {

		buff[0] = cval[0];
		buff[1] = cval[1];
		buff[2] = cval[2];
		buff[3] = cval[3];

	}
	else {

		buff[3] = cval[0];
		buff[2] = cval[1];
		buff[1] = cval[2];
		buff[0] = cval[3];

	}

}


void binary_from_data(const ResultData &data, std::vector<char> &buff) {

	// resize the buffer

	buff.resize(BinaryResultParser::MIN_BYTES + 2*4*data.glints.size());

	char *ptrBuff = buff.data();

	// insert the results into the buffer...

	// id
	UINT32_TO_4BYTE_LE(data.id, ptrBuff);
	ptrBuff += 4;


	// timestamp
	UINT32_TO_4BYTE_LE(data.timestamp, ptrBuff);
	ptrBuff += 4;


	// track duration
	UINT32_TO_4BYTE_LE(data.track_dur_micros, ptrBuff);
	ptrBuff += 4;


	// pupil ellipse
	FLOAT_TO_4BYTE_LE(data.pupil.center.x, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.pupil.center.y, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.pupil.size.width, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.pupil.size.height, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.pupil.angle, ptrBuff);
	ptrBuff += 4;


	// cornea centre
	FLOAT_TO_4BYTE_LE(data.cornea_centre.x, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.cornea_centre.y, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.cornea_centre.z, ptrBuff);
	ptrBuff += 4;


	// pupil centre
	FLOAT_TO_4BYTE_LE(data.pupil_centre.x, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.pupil_centre.y, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.pupil_centre.z, ptrBuff);
	ptrBuff += 4;


	// scene point
	FLOAT_TO_4BYTE_LE(data.scenePoint.x, ptrBuff);
	ptrBuff += 4;
	FLOAT_TO_4BYTE_LE(data.scenePoint.y, ptrBuff);
	ptrBuff += 4;


	// glints
	const std::vector<cv::Point2d> &crs = data.glints;
	size_t sz = crs.size();

	for(size_t i = 0; i < sz; ++i) {

		FLOAT_TO_4BYTE_LE(crs[i].x, ptrBuff);
		ptrBuff += 4;
		FLOAT_TO_4BYTE_LE(crs[i].y, ptrBuff);
		ptrBuff += 4;

	}

}


void printData(const ResultData &data) {

	printf("ID                   : %ld\n"
		   "timestamp            : %ld\n"
		   "track duration (us)  : %ld\n"
		   "pupil ellipse        : %.2f, %.2f, %.2f, %.2f, %.2f\n"
		   "cornea centre        : %.2f, %.2f, %.2f\n"
		   "pupil centre         : %.2f, %.2f, %.2f\n"
		   "scene point          : %.2f, %.2f\n"
		   "glints               : ",
			(long)data.id,
			(long)data.timestamp,
			(long)data.track_dur_micros,
			data.pupil.center.x,
			data.pupil.center.y,
			data.pupil.size.width,
			data.pupil.size.height,
			data.pupil.angle,
			data.cornea_centre.x,
			data.cornea_centre.y,
			data.cornea_centre.z,
			data.pupil_centre.x,
			data.pupil_centre.y,
			data.pupil_centre.z,
			data.scenePoint.x,
			data.scenePoint.y);

	const std::vector<cv::Point2d> &crs = data.glints;
	std::vector<cv::Point2d>::const_iterator it = crs.begin();
	std::vector<cv::Point2d>::const_iterator end = crs.end();

	while(it != end) {

		printf("(%.2f, %.2f) ", it->x, it->y);

		++it;

	}

	printf("\n");

}

