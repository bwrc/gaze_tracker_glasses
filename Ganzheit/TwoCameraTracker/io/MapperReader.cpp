#include "MapperReader.h"
#include <opencv2/core/core.hpp>


MapperReader::MapperReader() {

}


bool MapperReader::readContents(const std::string &file) {

	cv::FileStorage fs(file, cv::FileStorage::READ);
	if(!fs.isOpened()) {
		return false;
	}

	fs["cvM44SceneCam2VirtualEyeCam"] >> mappingMatrix;
	fs.release();


	return mappingMatrix.depth() == CV_64F;

}

