#ifndef CLUSTERISER_H
#define CLUSTERISER_H


#include <string.h>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>



namespace gt {


typedef std::vector<cv::Point> Cluster;


class Clusteriser {

	public:

		Clusteriser();
		~Clusteriser();

		void clusterise(const cv::Mat &image, const cv::Rect &rect_search);

		const std::vector<Cluster> &getClusters() const {
			return this->clusters;
		}

		const std::vector<int> &getHoleIndices() const {return ind_holes;}
		const std::vector<int> &getClusterIndices() const {return ind_clusters;}

		std::vector<cv::Vec4i> &getHierarchy() {return hierarchy;}

		void clearClusters();

	private:

		std::vector<Cluster> clusters;
		std::vector<int> ind_clusters;
		std::vector<int> ind_holes;

		std::vector<cv::Vec4i> hierarchy;
};


} // end of namespace gt {


#endif

