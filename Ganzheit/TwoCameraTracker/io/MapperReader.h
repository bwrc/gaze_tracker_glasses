#ifndef MAPPER_READER_H
#define MAPPER_READER_H


#include <string>
#include <opencv2/opencv.hpp>


/*
 * This class reads contents of a yaml -file, which describes
 * the transformation matrix between the eye and the scene
 * cameras.
 */


class MapperReader {

	public:

		MapperReader();

		bool readContents(const std::string &file);

		const cv::Mat &getTransformation() {return mappingMatrix;}

	private:

		/* 4x4 transformation matrix with double precision values */
		cv::Mat mappingMatrix;

};



#endif

