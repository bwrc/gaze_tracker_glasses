#include <cmath>
#include <iostream>

#include <Eigen/Core>

#include "Camera.h"

using namespace std;

int main(int argc, char *argv[])
{

	if(argc < 4) {
		cout << "Bad number of arguments (give me a 3d vector)" << endl;
	}

	Camera camera;

	/**********************************************************************
	 * Intrinsic and distortion matrixes.
	 *********************************************************************/

//////	double intrMat[9] = {991.473693, 0.000000,    650.251995,
//////		                          0.000000,   989.221843,  357.260790,
//////		                          0.000000,   0.000000,    1.000000};

	double intrMat[9] = {991.473693, 0.0, 0.0, 0.0, 989.221843, 0.0, 650.251995, 357.260790, 1.0};


	double distMat[5] = {0.018748,0.075206,0.000070,-0.003079,-0.296550};

	double distMat_zero_distortion[5] = {0.0,0.0,0.0,0.0,0.0};

	camera.setIntrinsicMatrix(intrMat);
	camera.setDistortion(distMat);

	// Determine the vector to be used in calculations.
	double original_vec3D[3] = {atof(argv[1]),atof(argv[2]),atof(argv[3])};
	double len = sqrt(original_vec3D[0] * original_vec3D[0] +
				original_vec3D[1] * original_vec3D[1] +
				original_vec3D[2] * original_vec3D[2]);

	original_vec3D[0] /= len;
	original_vec3D[1] /= len;
	original_vec3D[2] /= len;

	cout << "Converting vector " <<
		original_vec3D[0] << "," <<
		original_vec3D[1] << "," <<
		original_vec3D[2] <<
		" into display coordinate: " << endl;

	/**********************************************************************
	 * Convert the vector into display coordinate
	 *********************************************************************/

	double u = 0, v = 0;

	camera.worldToPix(original_vec3D, &u, &v);
	cout << "Display coordinate " << u << "," << v << endl;

	/**********************************************************************
	 * Determine the vector from display coordinate (use zero distortion
	 * matrix)
	 *********************************************************************/

	double vec3D[3];

	camera.setDistortion(distMat_zero_distortion);
	camera.pixToWorld(u, v, vec3D);
	cout << "Converting the coordinate back to 3d (without distortion correction):	" <<
		vec3D[0] << ",	" <<
		vec3D[1] << ",	" <<
		vec3D[2] <<
		"	Error: " <<
		vec3D[0] - original_vec3D[0] << ",	" <<
		vec3D[1] - original_vec3D[1] << ",	" <<
		vec3D[2] - original_vec3D[2] << endl;

	/**********************************************************************
	 * Determine the vector from display coordinate (use distortion matrix
	 * of the camera
	 *********************************************************************/

	camera.setDistortion(distMat);
	camera.pixToWorld(u, v, vec3D);
	cout << "Converting the coordinate back to 3d (with distortion correction):	" <<
		vec3D[0] << ",	" <<
		vec3D[1] << ",	" <<
		vec3D[2] <<
		"	Error:	" <<
		vec3D[0] - original_vec3D[0] << ",	" <<
		vec3D[1] - original_vec3D[1] << ",	" <<
		vec3D[2] - original_vec3D[2] << endl;

	return 0;
}
