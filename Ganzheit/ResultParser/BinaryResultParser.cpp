#include "BinaryResultParser.h"


// 2^32 - 1
static const uint32_t MAX_UINT32 = 4294967295;



/* Convert a 4-byte little-endian buffer to uint32 */
static uint32_t LE_4_BYTES_TO_UINT32(const char *buff) {

	uint32_t ret = (0x000000FF & buff[0])       |
				   (0x000000FF & buff[1]) << 8  |
				   (0x000000FF & buff[2]) << 16 |
				   (0x000000FF & buff[3]) << 24;

	return ret;

}


/* Convert a 2-byte little-endian buffer to uint16 */
static uint16_t LE_2_BYTES_TO_UINT16(const char *buff) {

	uint16_t ret = (0x00FF & buff[0])       |
				   (0x00FF & buff[1]) << 8;

	return ret;

}


/* Convert a 4-byte little-endian buffer to a 4-byte float */
static float LE_4_BYTES_TO_FLOAT(const char *buff) {

	uint32_t i = LE_4_BYTES_TO_UINT32(buff);

	float ret;
	memcpy(&ret, &i, 4);

	return ret;

}


/* Unsigned 4-byte integer to litte-endian 4-byte buffer */
static void UINT32_TO_4_BYTE_LE(uint32_t val, char *buff) {

	buff[0] = (val & 0x000000FF);
	buff[1] = (val & 0x0000FF00) >> 8;
	buff[2] = (val & 0x00FF0000) >> 16;
	buff[3] = (val & 0xFF000000) >> 24;

}


/* Unsigned 2-byte integer to litte-endian 2-byte buffer */
static void UINT16_TO_2_BYTE_LE(uint16_t val, char *buff) {

	buff[0] = (val & 0x00FF);
	buff[1] = (val & 0xFF00) >> 8;

}


/*
 * 4-byte floating point number to litte-endian 4-byte buffer
 */
static void FLOAT_TO_4_BYTE_LE(float val, char *buff, bool bIsLittleEndian) {

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


bool BinaryResultParser::parsePacket(const char *buff, const int len, ResultData &data) {

	data.clear();

	// sanity checks
	if(len < MIN_BYTES) {
		return false;
	}


	// define a pointer to the buffer
	const char *ptrBuff = buff;

	/*********************************************************************
	 * Data size 4 bytes
	 *********************************************************************/
	uint32_t sz = LE_4_BYTES_TO_UINT32(ptrBuff);
	ptrBuff += 4;

	if(sz != (uint32_t)len) {
		return false;
	}


	/*********************************************************************
	 * Success 1 byte
	 *********************************************************************/
	data.bTrackSuccessfull = *ptrBuff == 1;
	ptrBuff += 1;


	/*********************************************************************
	 * ID 4 bytes
	 *********************************************************************/
	data.id = LE_4_BYTES_TO_UINT32(ptrBuff);
	ptrBuff += 4;


	/*********************************************************************
	 * Timestamp 4 bytes
	 *********************************************************************/
	data.timestamp = LE_4_BYTES_TO_UINT32(ptrBuff);
	ptrBuff += 4;


	/*********************************************************************
	 * Track duration 4 bytes
	 *********************************************************************/
	data.trackDurMicros = LE_4_BYTES_TO_UINT32(ptrBuff);
	ptrBuff += 4;


	/*********************************************************************
	 * Pupil ellipse 5 * 4 bytes
	 *********************************************************************/
	data.ellipsePupil.center.x = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.ellipsePupil.center.y = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.ellipsePupil.size.width = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.ellipsePupil.size.height = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.ellipsePupil.angle = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;


	/*********************************************************************
	 * Cornea centre 3 * 4 bytes
	 *********************************************************************/
	data.corneaCentre.x = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.corneaCentre.y = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.corneaCentre.z = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;


	/*********************************************************************
	 * Pupil centre 3 * 4 bytes
	 *********************************************************************/
	data.pupilCentre.x = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.pupilCentre.y = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.pupilCentre.z = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;


	/*********************************************************************
	 * Scene point 2 * 4 bytes
	 *********************************************************************/
	data.scenePoint.x = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;
	data.scenePoint.y = LE_4_BYTES_TO_FLOAT(ptrBuff);
	ptrBuff += 4;


	/*********************************************************************
	 * Number of glints 2 bytes
	 *********************************************************************/
	int32_t nGlints = LE_2_BYTES_TO_UINT16(ptrBuff);
	ptrBuff += 2;


	/*********************************************************************
	 * Glints 2 * nGlints * 4 bytes
	 *********************************************************************/

	for(int i = 0; i < nGlints; ++i) {

		double x = LE_4_BYTES_TO_FLOAT(ptrBuff);
		ptrBuff += 4;
		double y = LE_4_BYTES_TO_FLOAT(ptrBuff);
		ptrBuff += 4;

		data.listGlints.push_back(cv::Point2d(x, y));

	}


	/*********************************************************************
	 * Number of contours 2 bytes
	 *********************************************************************/
	uint16_t nContours = LE_2_BYTES_TO_UINT16(ptrBuff);
	ptrBuff += 2;

	data.listContours.resize(nContours);


	/*********************************************************************
	 * Contours
	 *********************************************************************/
	for(int i = 0; i < nContours; ++i) {

		uint32_t nPoints = LE_4_BYTES_TO_UINT32(ptrBuff);
		ptrBuff += 4;

		std::vector<cv::Point> &curContour = data.listContours[i];
		curContour.resize(nPoints);

		for(unsigned int j = 0; j < nPoints; ++j) {

			uint16_t x = LE_2_BYTES_TO_UINT16(ptrBuff);
			ptrBuff += 2;
			uint16_t y = LE_2_BYTES_TO_UINT16(ptrBuff);
			ptrBuff += 2;

			curContour[j].x = x;
			curContour[j].y = y;

		}

	}


	/*********************************************************************
	 * Gaze vector start point in 2D, 2 * 2 bytes
	 *********************************************************************/
	data.gazeVecStartPoint2D.x = LE_2_BYTES_TO_UINT16(ptrBuff);
	ptrBuff += 2;
	data.gazeVecStartPoint2D.y = LE_2_BYTES_TO_UINT16(ptrBuff);
	ptrBuff += 2;


	/*********************************************************************
	 * Gaze vector end point in 2D, 2 * 2 bytes
	 *********************************************************************/
	data.gazeVecEndPoint2D.x = LE_2_BYTES_TO_UINT16(ptrBuff);
	ptrBuff += 2;
	data.gazeVecEndPoint2D.y = LE_2_BYTES_TO_UINT16(ptrBuff);
	ptrBuff += 2;


	return true;

}


/* Convert the data to a buffer */
void BinaryResultParser::resDataToBuffer(const ResultData &data, std::vector<char> &buff) {

	const uint16_t nContours = (uint16_t)data.listContours.size();

	int nContoursBytes = 4*nContours;
	for(size_t i = 0; i < nContours; ++i) {

		const std::vector<cv::Point> &curContour = data.listContours[i];

		nContoursBytes += 2*2*curContour.size();

	}

	const int nGlintBytes = 2*4*data.listGlints.size();
	const int resDataSz = MIN_BYTES + nGlintBytes + nContoursBytes;


	buff.resize(resDataSz);

	char *ptrBuff = buff.data();

	// insert the results into the buffer...

	// data size
	UINT32_TO_4_BYTE_LE(resDataSz, ptrBuff);
	ptrBuff += 4;


	// track success
	*ptrBuff = data.bTrackSuccessfull ? 1 : 0;
	ptrBuff += 1;


	// id
	UINT32_TO_4_BYTE_LE(data.id, ptrBuff);
	ptrBuff += 4;


	// timestamp
	UINT32_TO_4_BYTE_LE(data.timestamp, ptrBuff);
	ptrBuff += 4;


	// track duration
	UINT32_TO_4_BYTE_LE(data.trackDurMicros, ptrBuff);
	ptrBuff += 4;


	static const int a = 1;
	static const char *ca = (const char *)&a;
	static const bool bIsLittleEndian = ca[0] == 1;

	// pupil ellipse
	FLOAT_TO_4_BYTE_LE(data.ellipsePupil.center.x, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.ellipsePupil.center.y, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.ellipsePupil.size.width, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.ellipsePupil.size.height, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.ellipsePupil.angle, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;


	// cornea centre
	FLOAT_TO_4_BYTE_LE(data.corneaCentre.x, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.corneaCentre.y, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.corneaCentre.z, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;


	// pupil centre
	FLOAT_TO_4_BYTE_LE(data.pupilCentre.x, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.pupilCentre.y, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.pupilCentre.z, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;


	// scene point
	FLOAT_TO_4_BYTE_LE(data.scenePoint.x, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;
	FLOAT_TO_4_BYTE_LE(data.scenePoint.y, ptrBuff, bIsLittleEndian);
	ptrBuff += 4;


	// number of glints
	int16_t nGlints = (int16_t)data.listGlints.size();
	UINT16_TO_2_BYTE_LE(nGlints, ptrBuff);
	ptrBuff += 2;


	// glints
	const std::vector<cv::Point2d> &crs = data.listGlints;

	for(int i = 0; i < nGlints; ++i) {

		FLOAT_TO_4_BYTE_LE(crs[i].x, ptrBuff, bIsLittleEndian);
		ptrBuff += 4;
		FLOAT_TO_4_BYTE_LE(crs[i].y, ptrBuff, bIsLittleEndian);
		ptrBuff += 4;

	}


	// number of contours
	UINT16_TO_2_BYTE_LE(nContours, ptrBuff);
	ptrBuff += 2;

	for(size_t i = 0; i < nContours; ++i) {

		const std::vector<cv::Point> &curContour = data.listContours[i];

		uint32_t nPoints = (uint32_t)curContour.size();
		UINT32_TO_4_BYTE_LE(nPoints, ptrBuff);
		ptrBuff += 4;

		for(uint32_t j = 0; j < nPoints; ++j) {

			const cv::Point &curPoint = curContour[j];

			UINT16_TO_2_BYTE_LE(curPoint.x, ptrBuff);
			ptrBuff += 2;
			UINT16_TO_2_BYTE_LE(curPoint.y, ptrBuff);
			ptrBuff += 2;

		}

	}


	// start point of the gaze vector in 2D
	UINT16_TO_2_BYTE_LE(data.gazeVecStartPoint2D.x, ptrBuff);
	ptrBuff += 2;
	UINT16_TO_2_BYTE_LE(data.gazeVecStartPoint2D.y, ptrBuff);
	ptrBuff += 2;


	// end point of the gaze vector in 2D
	UINT16_TO_2_BYTE_LE(data.gazeVecEndPoint2D.x, ptrBuff);
	ptrBuff += 2;
	UINT16_TO_2_BYTE_LE(data.gazeVecEndPoint2D.y, ptrBuff);
	ptrBuff += 2;

}

