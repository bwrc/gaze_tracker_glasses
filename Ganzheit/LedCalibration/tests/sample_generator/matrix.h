#ifndef MATRIX_H
#define MATRIX_H

#include <string.h>
#include <stdio.h>
#include <math.h>


static const double PI	= 3.1415926535897932384626433832795;


static inline double RADTODEG(double rad) {
	return 180.0 * rad / PI;
}


static inline double DEGTORAD(double deg) {
	return deg * PI / 180.0;
}


/* column-major */
static void setIdentity(double glMat[16]) {

	memset(glMat, 0, 16*sizeof(double));
	glMat[0]	= 1.0;
	glMat[5]	= 1.0;
	glMat[10]	= 1.0;
	glMat[15]	= 1.0;

}


static void multMatVec(const double m[16], const double v[4], double res[4]) {

	res[0] = m[0] * v[0] + m[4] * v[1] + m[8]  * v[2] + m[12] * v[3];
	res[1] = m[1] * v[0] + m[5] * v[1] + m[9]  * v[2] + m[13] * v[3];
	res[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
	res[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];

}


/* column-major */
static void multMatSafe(const double m1[16], const double m2[16], double matRes[16]) {

	double tmpRes[16];

	for(int i = 0; i < 4; ++i) {

		const int i1 = 0  + i;
		const int i2 = 4  + i;
		const int i3 = 8  + i;
		const int i4 = 12 + i;

		const double m11 = m1[i1];
		const double m12 = m1[i2];
		const double m13 = m1[i3];
		const double m14 = m1[i4];


		// (i, 0)
		tmpRes[i1] = m11 * m2[0]  +
					 m12 * m2[1]  +
					 m13 * m2[2]  +
					 m14 * m2[3];

		// (i, 1)
		tmpRes[i2] = m11 * m2[4]  +
					 m12 * m2[5]  +
					 m13 * m2[6]  +
					 m14 * m2[7];

		// (i, 2)
		tmpRes[i3] = m11 * m2[8]  +
					 m12 * m2[9]  +
					 m13 * m2[10] +
					 m14 * m2[11];

		// (i, 3)
		tmpRes[i4] = m11 * m2[12] +
					 m12 * m2[13] +
					 m13 * m2[14] +
					 m14 * m2[15];

	}

	memcpy(matRes, tmpRes, 16*sizeof(double));

}


static void printMat(double mat[16]) {

	printf("\n*********** Matix ************\n");

	// row 1
	printf("%.2f, %.2f, %.2f, %.2f\n", mat[0], mat[4], mat[8], mat[12]);

	// row 2
	printf("%.2f, %.2f, %.2f, %.2f\n", mat[1], mat[5], mat[9], mat[13]);

	// row 3
	printf("%.2f, %.2f, %.2f, %.2f\n", mat[2], mat[6], mat[10], mat[14]);

	// row 4
	printf("%.2f, %.2f, %.2f, %.2f\n", mat[3], mat[7], mat[11], mat[15]);

}


static void makeXRotMat(double mat[16], double angDeg) {

	double ang_rad = DEGTORAD(angDeg);
	double cos_ang = cos(ang_rad);
	double sin_ang = sin(ang_rad);

	// column 1
	mat[0]	= 1.0;
	mat[1]	= 0.0;
	mat[2]	= 0.0;
	mat[3]	= 0.0;

	// column 2
	mat[4]	= 0.0;
	mat[5]	= cos_ang;
	mat[6]	= sin_ang;
	mat[7]	= 0.0;

	// column 3
	mat[8]	= 0.0;
	mat[9]	= -sin_ang;
	mat[10]	= cos_ang;
	mat[11]	= 0.0;

	// column 4
	mat[12]	= 0.0;
	mat[13]	= 0.0;
	mat[14]	= 0.0;
	mat[15]	= 1.0;

}


static void makeYRotMat(double mat[16], double angDeg) {

	double ang_rad = DEGTORAD(angDeg);
	double cos_ang = cos(ang_rad);
	double sin_ang = sin(ang_rad);

	// column 1
	mat[0]	= cos_ang;
	mat[1]	= 0.0;
	mat[2]	= -sin_ang;
	mat[3]	= 0.0;

	// column 2
	mat[4]	= 0.0;
	mat[5]	= 1.0;
	mat[6]	= 0.0;
	mat[7]	= 0.0;

	// column 3
	mat[8]	= sin_ang;
	mat[9]	= 0.0;
	mat[10]	= cos_ang;
	mat[11]	= 0.0;

	// column 4
	mat[12]	= 0.0;
	mat[13]	= 0.0;
	mat[14]	= 0.0;
	mat[15]	= 1.0;

}


static void makeZRotMat(double mat[16], double angDeg) {

	double ang_rad = DEGTORAD(angDeg);
	double cos_ang = cos(ang_rad);
	double sin_ang = sin(ang_rad);

	// column 1
	mat[0]	= cos_ang;
	mat[1]	= sin_ang;
	mat[2]	= 0.0;
	mat[3]	= 0.0;

	// column 2
	mat[4]	= -sin_ang;
	mat[5]	= cos_ang;
	mat[6]	= 0.0;
	mat[7]	= 0.0;

	// column 3
	mat[8]	= 0.0;
	mat[9]	= 0.0;
	mat[10]	= 1.0;
	mat[11]	= 0.0;

	// column 4
	mat[12]	= 0.0;
	mat[13]	= 0.0;
	mat[14]	= 0.0;
	mat[15]	= 1.0;

}


static double dot(double v1[3], double v2[3]) {

	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

}


static double vecToVecDist(double v1[3], double v2[3]) {

	double dx = v2[0] - v1[0];
	double dy = v2[1] - v1[1];
	double dz = v2[2] - v1[2];

	return sqrt(dx*dx + dy*dy + dz*dz);

}


static void normalise(double v[3], double vn[3]) {

	double x = v[0];
	double y = v[1];
	double z = v[2];

	double d = sqrt(x*x + y*y + z*z);
	vn[0] = x / d;
	vn[1] = y / d;
	vn[2] = z / d;

}


static void normalise(double v[3]) {

	double x = v[0];
	double y = v[1];
	double z = v[2];

	double d = sqrt(x*x + y*y + z*z);
	v[0] = x / d;
	v[1] = y / d;
	v[2] = z / d;

}



/* http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix */
static bool invert(const double m[16], double invOut[16]) {

	double inv[16];

	inv[0] = m[5]  * m[10] * m[15] - 
			 m[5]  * m[11] * m[14] - 
			 m[9]  * m[6]  * m[15] + 
			 m[9]  * m[7]  * m[14] +
			 m[13] * m[6]  * m[11] - 
			 m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] + 
			  m[4]  * m[11] * m[14] + 
			  m[8]  * m[6]  * m[15] - 
			  m[8]  * m[7]  * m[14] - 
			  m[12] * m[6]  * m[11] + 
			  m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9] * m[15] - 
			 m[4]  * m[11] * m[13] - 
			 m[8]  * m[5] * m[15] + 
			 m[8]  * m[7] * m[13] + 
			 m[12] * m[5] * m[11] - 
			 m[12] * m[7] * m[9];

	inv[12] = -m[4]  * m[9] * m[14] + 
			   m[4]  * m[10] * m[13] +
			   m[8]  * m[5] * m[14] - 
			   m[8]  * m[6] * m[13] - 
			   m[12] * m[5] * m[10] + 
			   m[12] * m[6] * m[9];

	inv[1] = -m[1]  * m[10] * m[15] + 
			  m[1]  * m[11] * m[14] + 
			  m[9]  * m[2] * m[15] - 
			  m[9]  * m[3] * m[14] - 
			  m[13] * m[2] * m[11] + 
			  m[13] * m[3] * m[10];

	inv[5] = m[0]  * m[10] * m[15] - 
			  m[0]  * m[11] * m[14] - 
			  m[8]  * m[2] * m[15] + 
			  m[8]  * m[3] * m[14] + 
			  m[12] * m[2] * m[11] - 
			  m[12] * m[3] * m[10];

	inv[9] = -m[0]  * m[9] * m[15] + 
			  m[0]  * m[11] * m[13] + 
			  m[8]  * m[1] * m[15] - 
			  m[8]  * m[3] * m[13] - 
			  m[12] * m[1] * m[11] + 
			  m[12] * m[3] * m[9];

	inv[13] = m[0]  * m[9] * m[14] - 
			  m[0]  * m[10] * m[13] - 
			  m[8]  * m[1] * m[14] + 
			  m[8]  * m[2] * m[13] + 
			  m[12] * m[1] * m[10] - 
			  m[12] * m[2] * m[9];

	inv[2] = m[1]  * m[6] * m[15] - 
			 m[1]  * m[7] * m[14] - 
			 m[5]  * m[2] * m[15] + 
			 m[5]  * m[3] * m[14] + 
			 m[13] * m[2] * m[7] - 
			 m[13] * m[3] * m[6];

	inv[6] = -m[0]  * m[6] * m[15] + 
			  m[0]  * m[7] * m[14] + 
			  m[4]  * m[2] * m[15] - 
			  m[4]  * m[3] * m[14] - 
			  m[12] * m[2] * m[7] + 
			  m[12] * m[3] * m[6];

	inv[10] = m[0]  * m[5] * m[15] - 
			  m[0]  * m[7] * m[13] - 
			  m[4]  * m[1] * m[15] + 
			  m[4]  * m[3] * m[13] + 
			  m[12] * m[1] * m[7] - 
			  m[12] * m[3] * m[5];

	inv[14] = -m[0]  * m[5] * m[14] + 
			  m[0]  * m[6] * m[13] + 
			  m[4]  * m[1] * m[14] - 
			  m[4]  * m[2] * m[13] - 
			  m[12] * m[1] * m[6] + 
			  m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] + 
			  m[1] * m[7] * m[10] + 
			  m[5] * m[2] * m[11] - 
			  m[5] * m[3] * m[10] - 
			  m[9] * m[2] * m[7] + 
			  m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] - 
			  m[0] * m[7] * m[10] - 
			  m[4] * m[2] * m[11] + 
			  m[4] * m[3] * m[10] + 
			  m[8] * m[2] * m[7] - 
			  m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] + 
			  m[0] * m[7] * m[9] + 
			  m[4] * m[1] * m[11] - 
			  m[4] * m[3] * m[9] - 
			  m[8] * m[1] * m[7] + 
			  m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] - 
			  m[0] * m[6] * m[9] - 
			  m[4] * m[1] * m[10] + 
			  m[4] * m[2] * m[9] + 
			  m[8] * m[1] * m[6] - 
			  m[8] * m[2] * m[5];

	double det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if(det == 0) {
	return false;
	}

	det = 1.0 / det;

	for(int i = 0; i < 16; i++) {
	invOut[i] = inv[i] * det;
	}

	return true;

}


#endif

