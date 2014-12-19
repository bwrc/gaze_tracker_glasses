#include <cstdio>

#include "SceneTracker.h"
#include "svd.h"

bool SceneTracker::calibrate(const std::vector<CalibrationPoint>& points)
{

	if(points.size() != 9)
		return false;

	/* Transform calibration points to calibration vectors where
	 * the eye is represented as a vector from eye to glint (this should
	 * increase the accuracy of track) */

	std::vector<CalibrationVector> vec_points;
	for(std::vector<CalibrationPoint>::const_iterator it = points.begin();
		it != points.end();
		it++) {

		vec_points.push_back(CalibrationVector(*it));
	}

	// Calibrate
	cal_calibration_homography(vec_points);
	calibrationComplete = true;

	return true;
}

cv::Point2f SceneTracker::track(const cv::Point2f& eyePoint, const cv::Point2f& glintPoint)
{
	cv::Point2f eyeVector = eyePoint - glintPoint;
	cv::Point2i p2(0, 0);

	if(!this->calibrationComplete)
		return p2;

	double z = map_matrix[2][0]*eyeVector.x + map_matrix[2][1]*eyeVector.y + map_matrix[2][2];
	p2.x = (int)((map_matrix[0][0]*eyeVector.x + map_matrix[0][1]*eyeVector.y + map_matrix[0][2])/z);
	p2.y = (int)((map_matrix[1][0]*eyeVector.x + map_matrix[1][1]*eyeVector.y + map_matrix[1][2])/z);

	return p2;

}

void SceneTracker::cal_calibration_homography(const std::vector<CalibrationVector>& calibrationPoints)
{
	int i, j;

	double dis_scale_scene, dis_scale_eye;
	cv::Point2f scene_center, eye_center;
	std::vector<CalibrationVector> normalizedPoints;

	normalize_point_set(calibrationPoints, normalizedPoints, dis_scale_eye, dis_scale_scene, eye_center, scene_center);

	printf("normalize_point_set end\n");
	printf("scene scale:%.2f  center (%.2f, %.2f)\n", dis_scale_scene, scene_center.x, scene_center.y);
	printf("eye scale:%.2f  center (%.2f, %.2f)\n", dis_scale_eye, eye_center.x, eye_center.y);

	const int homo_row=18, homo_col=9;
	double A[homo_row][homo_col];
	int M = homo_row, N = homo_col; //M is row; N is column
	double **ppa = new double*[M]; //(double**)malloc(sizeof(double*)*M);
	double **ppu = new double*[M]; //(double**)malloc(sizeof(double*)*M);
	double **ppv = new double*[N]; //(double**)malloc(sizeof(double*)*N);
	double pd[homo_col];

	for (i = 0; i < M; i++) {
		ppa[i] = A[i];
		ppu[i] = new double[N]; //(double*)malloc(sizeof(double)*N);
	}

	for (i = 0; i < N; i++) {
		ppv[i] = new double[N]; //(double*)malloc(sizeof(double)*N);
	}

	for (j = 0;  j< M; j++) {
		if (j%2 == 0) {
			A[j][0] = A[j][1] = A[j][2] = 0;
			A[j][3] = -normalizedPoints[j/2].eyeVector.x;
			A[j][4] = -normalizedPoints[j/2].eyeVector.y;
			A[j][5] = -1;
			A[j][6] = normalizedPoints[j/2].scenePoint.y * normalizedPoints[j/2].eyeVector.x;
			A[j][7] = normalizedPoints[j/2].scenePoint.y * normalizedPoints[j/2].eyeVector.y;
			A[j][8] = normalizedPoints[j/2].scenePoint.y;
		} else {
			A[j][0] = normalizedPoints[j/2].eyeVector.x;
			A[j][1] = normalizedPoints[j/2].eyeVector.y;
			A[j][2] = 1;
			A[j][3] = A[j][4] = A[j][5] = 0;
			A[j][6] = -normalizedPoints[j/2].scenePoint.x * normalizedPoints[j/2].eyeVector.x;
			A[j][7] = -normalizedPoints[j/2].scenePoint.x * normalizedPoints[j/2].eyeVector.y;
			A[j][8] = -normalizedPoints[j/2].scenePoint.x;
		}
	}

	printf("normalize_point_set end\n");

	svd(M, N, ppa, ppu, pd, ppv);
	int min_d_index = 0;
	for (i = 1; i < N; i++) {
		if (pd[i] < pd[min_d_index])
			min_d_index = i;
	}

	for (i = 0; i < N; i++) {

		/* the column of v that corresponds to the smallest singular
		 * value, which is the solution of the equations */

		this->map_matrix[i/3][i%3] = ppv[i][min_d_index]; 	
	}

	double T[3][3] = {{0}}, T1[3][3] = {{0}};
	printf("\nT1: \n");
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			printf("%.2f ", T1[j][i]);
		}
		printf("\n");
	}  

	T[0][0] = T[1][1] = dis_scale_eye;
	T[0][2] = -dis_scale_eye*eye_center.x;
	T[1][2] = -dis_scale_eye*eye_center.y;
	T[2][2] = 1;

	printf("\nmap_matrix: \n");
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			printf("%.2f ", this->map_matrix[j][i]);
		}
		printf("\n");
	}   
	printf("\nT: \n");
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			printf("%.2f ", T[j][i]);
		}
		printf("\n");
	}  

	matrix_multiply33(this->map_matrix, T, this->map_matrix); 

	T[0][0] = T[1][1] = dis_scale_scene;
	T[0][2] = -dis_scale_scene*scene_center.x;
	T[1][2] = -dis_scale_scene*scene_center.y;
	T[2][2] = 1;

	printf("\nmap_matrix: \n");
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			printf("%.2f ", this->map_matrix[j][i]);
		}
		printf("\n");
	} 
	printf("\nT: \n");
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			printf("%.2f ", T[j][i]);
		}
		printf("\n");
	}   

	affine_matrix_inverse(T, T1);
	matrix_multiply33(T1, this->map_matrix, this->map_matrix);

	printf("\nmap_matrix: \n");
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			printf("%.2f ", this->map_matrix[j][i]);
		}
		printf("\n");
	}   

	for (i = 0; i < M; i++) {
		delete ppu[i]; //free(ppu[i]);
	}
	for (i = 0; i < N; i++) {
		delete ppv[i]; //free(ppv[i]);
	}
	delete ppu; //free(ppu);
	delete ppv; //free(ppv);
	delete ppa; //free(ppa);

	printf("\nfinish calculate calibration\n");
}

void SceneTracker::normalize_point_set(const std::vector<CalibrationVector>& point_set,
			std::vector<CalibrationVector>& norm_point_set,
			double &dis_scale_eye, double& dis_scale_scene, cv::Point2f& nor_center_eye,
			cv::Point2f& nor_center_scene)

{
	cv::Point2f sum_eye, sum_scene;
	double sumdis_eye = 0, sumdis_scene = 0;
	int num = point_set.size();

	// Compute the vector sum and the total length of all vectors
	std::vector<CalibrationVector>::const_iterator edge = point_set.begin();
	for (; edge != point_set.end(); edge++) {
		sum_eye += edge->eyeVector;
		sumdis_eye += sqrt((double)(edge->eyeVector.x*edge->eyeVector.x + edge->eyeVector.y*edge->eyeVector.y));

		sum_scene += edge->scenePoint;
		sumdis_scene += sqrt((double)(edge->scenePoint.x*edge->scenePoint.x + edge->scenePoint.y*edge->scenePoint.y));

	}

	dis_scale_eye = sqrt(2.0 * (double)num / (double)sumdis_eye);
	nor_center_eye = sum_eye; nor_center_eye.x /= (double)num; nor_center_eye.y /= (double)num;

	dis_scale_scene = sqrt(2.0 * (double)num / (double)sumdis_scene);
	nor_center_scene = sum_scene; nor_center_scene.x /= (double)num; nor_center_scene.y /= (double)num;

	norm_point_set.clear();

	// Normalize both eye vectors and scene points
	for (edge = point_set.begin(); edge != point_set.end(); edge++) {

		cv::Point2f normalized_point_eye = cv::Point2f(((edge->eyeVector.x - nor_center_eye.x) * dis_scale_eye),
			((edge->eyeVector.y - nor_center_eye.y) * dis_scale_eye));
		cv::Point2f normalized_point_scene = cv::Point2f(((edge->scenePoint.x - nor_center_scene.x) *
			dis_scale_scene), ((edge->scenePoint.y - nor_center_scene.y) * dis_scale_scene));

		norm_point_set.push_back(CalibrationVector(normalized_point_eye, normalized_point_scene));
	}

}

// r is result matrix
void SceneTracker::affine_matrix_inverse(double a[][3], double r[][3])
{
	double det22 = a[0][0]*a[1][1] - a[0][1]*a[1][0];
	r[0][0] = a[1][1]/det22;
	r[0][1] = -a[0][1]/det22;
	r[1][0] = -a[1][0]/det22;
	r[1][1] = a[0][0]/det22;

	r[2][0] = r[2][1] = 0;
	r[2][2] = 1/a[2][2];

	r[0][2] = -r[2][2] * (r[0][0]*a[0][2] + r[0][1]*a[1][2]);
	r[1][2] = -r[2][2] * (r[1][0]*a[0][2] + r[1][1]*a[1][2]);
}

// r is result matrix
void SceneTracker::matrix_multiply33(double a[][3], double b[][3], double r[][3])
{
	int i, j;
	double result[9];
	double v = 0;

	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			v = a[j][0]*b[0][i];
			v += a[j][1]*b[1][i];
			v += a[j][2]*b[2][i];
			result[j*3+i] = v;
		}
	}

	for (i = 0; i < 3; i++) {
		r[i][0] = result[i*3];
		r[i][1] = result[i*3+1];
		r[i][2] = result[i*3+2];
	}
}

