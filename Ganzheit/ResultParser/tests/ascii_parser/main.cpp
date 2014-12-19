#include "ResultParser.h"
#include <stdio.h>
#include <sys/time.h>


void string_from_data(const ResultData &data, char *str);
void printData(const ResultData &data);


int main(int nof_args, const char **list_args) {

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
	char *buff = new char[1024];
	string_from_data(orig_data, buff);
	printf("%s\n", buff);

	printf("*****************************************\n"
		   "* Original data:\n"
		   "*****************************************\n");
	for(size_t i = 0; i < strlen(buff); ++i) {
		putchar(buff[i]);
	}
	printf("\n\n\n");


	/***************************************************
	 * Parse the buffer into data
	 ***************************************************/
	struct timeval t1, t2;
	gettimeofday(&t1, 0);
	ResultData data;

	if(!ResultParser::parsePacket(buff, strlen(buff), data)) {

		printf("*****************************************\n"
			   "* !!! Could not parse the packet !!!\n"
			   "*****************************************\n");

	}
	else {

		gettimeofday(&t2, 0);

		long sec				= t2.tv_sec - t1.tv_sec;
		long micros_tmp			= t2.tv_usec - t1.tv_usec;
		unsigned long micros	= (unsigned long)(1000000.0 * sec + micros_tmp);

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

	delete[] buff;

}


void string_from_data(const ResultData &data, char *str) {

	int pos = 0;

	// insert the results into the buffer...

	// id
	char tmp[512];
	sprintf(tmp, "%ld, ", (long)data.id);
	int tmplen = strlen(tmp);
	memcpy(str, tmp, tmplen);
	pos += tmplen;


	// time
	sprintf(tmp, "%ld, ", (long)data.timestamp);
	tmplen = strlen(tmp);
	memcpy(str + pos, tmp, tmplen);
	pos += tmplen;


	// track duration
	sprintf(tmp, "%ld, ", data.track_dur_micros);
	tmplen = strlen(tmp);
	memcpy(str + pos, tmp, tmplen);
	pos += tmplen;


	// pupil ellipse
	const cv::RotatedRect &pupil = data.pupil;
	sprintf(tmp, "[%.2f, %.2f, %.2f, %.2f, %.2f], ", pupil.center.x,
												   pupil.center.y,
												   pupil.size.width,
												   pupil.size.height,
												   pupil.angle);
	tmplen = strlen(tmp);
	memcpy(str + pos, tmp, tmplen);
	pos += tmplen;

	// cornea centre
	const cv::Point3d &cc = data.cornea_centre;
	sprintf(tmp, "[%.2f, %.2f, %.2f],", cc.x, cc.y, cc.z);
	tmplen = strlen(tmp);
	memcpy(str + pos, tmp, tmplen);
	pos += tmplen;


	// pupil centre
	const cv::Point3d &pc = data.pupil_centre;
	sprintf(tmp, "[%.2f, %.2f, %.2f],", pc.x, pc.y, pc.z);
	tmplen = strlen(tmp);
	memcpy(str + pos, tmp, tmplen);
	pos += tmplen;


	// scene point
	const cv::Point2d &sp = data.scenePoint;
	sprintf(tmp, "[%.2f, %.2f],", sp.x, sp.y);
	tmplen = strlen(tmp);
	memcpy(str + pos, tmp, tmplen);
	pos += tmplen;


	// glints
	const std::vector<cv::Point2d> &crs = data.glints;
	size_t sz = crs.size();
	tmp[0] = '\0';
	for(size_t i = 0; i < sz; ++i) {

		char tmp2[128];
		memset(tmp2, '\0', 128);

		sprintf(tmp2, "[%.2f, %.2f],", crs[i].x, crs[i].y);
		int len = strlen(tmp2);

		memcpy(str + pos, tmp2, len);
		pos += len;

	}


	str[pos-1] = '\0';
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

	while(it != crs.end()) {

		printf("(%.2f, %.2f) ", it->x, it->y);

		++it;

	}

	printf("\n");

}

