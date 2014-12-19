#include <cmath>
#include <iostream>


#include "Camera.h"

using namespace std;

int main(int argc, char *argv[])
{

	Camera camera;

	/**********************************************************************
	 * Intrinsic and distortion matrixes.
	 *********************************************************************/

//////	double intrMat[9] = {1007.403041, 0.000000,    657.009401,
//////						 0.000000,    1003.985629, 361.350078,
//////						 0.000000,    0.000000,    1.000000};


	double intrMat[9] = {1007.403041, 0.0, 0.0, 0.0, 1003.985629, 0.0, 657.009401, 361.350078, 1.0};
	double distMat[5] = {0.0};//{0.014448, 0.172008, 0.000534, -0.000992, -0.589497};

	camera.setIntrinsicMatrix(intrMat);
	camera.setDistortion(distMat);



	/**********************************************************************
	 * Convert the vector into display coordinate
	 *********************************************************************/

	cv::Point3d p3D;
	double u = 640, v = 360;
	camera.pixToWorld(u, v, p3D);

	cout << endl;
	cout << "****************************************************************" << endl;
	cout << "                         Pixel to world                         " << endl;
	cout << "Display coordinate " << u << ", " << v << endl;
	cout << "World coordinate   " << p3D.x << ", " << p3D.y << ", " << p3D.z << endl;
	cout << "****************************************************************" << endl << endl;

	/**********************************************************************
	 * Determine the vector from display coordinate (use zero distortion
	 * matrix)
	 *********************************************************************/

	camera.worldToPix(p3D, &u, &v);

	cout << endl;
	cout << "****************************************************************" << endl;
	cout << "                         World to pixel                         " << endl;
	cout << "World coordinate   " << p3D.x << ", " << p3D.y << ", " << p3D.z << endl;
	cout << "Display coordinate " << u << ", " << v << endl;
	cout << "****************************************************************" << endl << endl;


	/**********************************************************************
	 * Determine the vector from display coordinate (use distortion matrix
	 * of the camera
	 *********************************************************************/

//	camera.setDistortion(distMat);
//	camera.pixToWorld(u, v, vec3D);
//	cout << "Converting the coordinate back to 3d (with distortion correction):	" <<
//		vec3D[0] << ",	" <<
//		vec3D[1] << ",	" <<
//		vec3D[2] <<
//		"	Error:	" <<
//		vec3D[0] - original_vec3D[0] << ",	" <<
//		vec3D[1] - original_vec3D[1] << ",	" <<
//		vec3D[2] - original_vec3D[2] << endl;

	return 0;
}
