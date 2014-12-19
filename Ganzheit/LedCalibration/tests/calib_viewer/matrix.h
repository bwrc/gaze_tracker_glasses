#ifndef MATRIX_H
#define MATRIX_H


const double PI = 3.1415926535897932384626433832795;


inline double RADTODEG(double rad);
inline double DEGTORAD(double deg);


/* column-major */
void setIdentity(double glMat[16]);

void multMatVec(const double m[16], const double v[4], double res[4]);


/* column-major */
void multMatSafe(const double m1[16], const double m2[16], double matRes[16]);


void printMat(double mat[16]);void makeXRotMat(double mat[16], double angDeg);


void makeYRotMat(double mat[16], double angDeg);

void makeZRotMat(double mat[16], double angDeg);

double dot(double v1[3], double v2[3]);

double vecToVecDist(double v1[3], double v2[3]);

// http://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
double pointToLineDistance(const double A[3], const double B[3], const double P[3]);

void normalise(double v[3], double vn[3]);

void normalise(double v[3]);

/* http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix */
bool invert(const double m[16], double invOut[16]);


#endif

