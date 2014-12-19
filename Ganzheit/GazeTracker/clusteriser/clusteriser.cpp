#include "clusteriser.h"


namespace gt {


Clusteriser::Clusteriser() {}


Clusteriser::~Clusteriser() {
	clearClusters();
}


void Clusteriser::clearClusters() {

	for(int i = 0; i < (int)clusters.size(); ++i) {

		clusters[i].clear();

	}

	clusters.clear();

	ind_clusters.clear();
	ind_holes.clear();

	hierarchy.clear();

}


void Clusteriser::clusterise(const cv::Mat &image_binary, const cv::Rect &rect_search) {

    // clear previous clusters
	clearClusters();

	/********************************************************************************************
	 * Based on finding the clusters and holes
	 ********************************************************************************************/

	// a matrix header to the image with the given ROI
	const cv::Mat img_roi(image_binary, rect_search);

	// clone because findContours modifies the image
	cv::Mat tmp = img_roi.clone();

	// find the contours
	cv::findContours(tmp, this->clusters, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cv::Point(rect_search.x, rect_search.y));

	if(clusters.size() == 0) {
		return;
	}

	std::vector<int> tmp_ind(clusters.size());

	for(size_t i = 0; i < tmp_ind.size(); ++i) {
		tmp_ind[i] = i;
	}

	// populate the cluster indices
	for(int idx = 0; idx >= 0; idx = hierarchy[idx][0]) {
		ind_clusters.push_back(idx);
		tmp_ind[idx] = -1;
	}


	ind_holes.resize(clusters.size() - ind_clusters.size());

	int c = 0;
	for(size_t i = 0; i < tmp_ind.size(); ++i) {
		if(tmp_ind[i] != -1) {
			ind_holes[c++] = tmp_ind[i];
		}
	}

}


}	// end of namespace gt {

