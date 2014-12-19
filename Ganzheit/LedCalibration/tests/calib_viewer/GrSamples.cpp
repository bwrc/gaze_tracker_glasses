#include "GrSamples.h"
#include "matrix.h"


static const double LED_SIDE_LENGTH = 0.001;


GrSamples::GrSamples() {

    memset(glintOGLPoint, 0, 3*sizeof(double));
    memset(patternOGLPoints, 0, 3*16*sizeof(double));
    memset(vecReflection, 0, 3*sizeof(double));
    memset(vecIntersection, 0, 3*sizeof(double));
    memset(posLED, 0, 3*sizeof(double));

}


void GrSamples::reset(const calib::LEDCalibContainer &container,
                      const calib::LEDCalibSample &sample,
                      const Camera &cam) {

    posLED[0] =  container.LED_pos[0];
    posLED[1] = -container.LED_pos[1];
    posLED[2] = -container.LED_pos[2];


    // get the object points
    std::vector<cv::Point3f> objectPoints;
    calib::LEDCalibPattern::getLEDObjectPoints(container.circleSpacing, objectPoints);

    Eigen::Matrix4d tm;
    calib::LEDCalibPattern::makeTransformationMatrix(cam.getIntrisicMatrix(),
                                                     cam.getDistortion(),
                                                     sample.image_points,
                                                     objectPoints,
                                                     tm);

    computePatternOGLPoints(objectPoints,
                            tm);

    computeGlintOGLPoint(cam,
                         tm,
                         objectPoints,
                         sample);

    computeRays(cam,
                sample,
                container.circleSpacing);


     double *a = vecIntersection;
     double b[3] = {a[0] + vecReflection[0],
                    a[1] + vecReflection[1],
                    a[2] + vecReflection[2]};

     double dist = pointToLineDistance(a, b, posLED);

     printf("    distance = %.2fmm\n", dist*1000.0);

}


void GrSamples::computePatternOGLPoints(const Camera &cam,
                                        const std::vector<cv::Point2f> &imagePoints,
                                        double circleSpacing) {

    // get the object points
    std::vector<cv::Point3f> objectPoints;
    calib::LEDCalibPattern::getLEDObjectPoints(circleSpacing, objectPoints);

    Eigen::Matrix4d tm;
    calib::LEDCalibPattern::makeTransformationMatrix(cam.getIntrisicMatrix(),
                                                     cam.getDistortion(),
                                                     imagePoints,
                                                     objectPoints,
                                                     tm);

    size_t sz = objectPoints.size(); // must be 16


    computePatternOGLPoints(objectPoints,
                            tm);

}


void GrSamples::computePatternOGLPoints(const std::vector<cv::Point3f> &objectPoints,
                                        const Eigen::Matrix4d &tm) {

    size_t sz = objectPoints.size(); // must be 16

	for(size_t i = 0; i < sz; ++i) {

        // get the current point
        const cv::Point3f &curPoint = objectPoints[i];

        /*
         * Compute the transformed point. These are in the camera coord.
         * system.
         */
        double ocvCamX =
            tm(0, 0) * curPoint.x +
            tm(0, 1) * curPoint.y +
            tm(0, 2) * curPoint.z +
            tm(0, 3) * 1.0f;

        double ocvCamY =
            tm(1, 0) * curPoint.x +
            tm(1, 1) * curPoint.y +
            tm(1, 2) * curPoint.z +
            tm(1, 3) * 1.0f;

        double ocvCamZ =
            tm(2, 0) * curPoint.x +
            tm(2, 1) * curPoint.y +
            tm(2, 2) * curPoint.z +
            tm(2, 3) * 1.0f;

        double *p = patternOGLPoints + 3*i;

        *p++ =  ocvCamX;
        *p++ = -ocvCamY;
        *p   = -ocvCamZ;

    }

}


void GrSamples::computeGlintOGLPoint(const Camera &cam,
                                     const calib::LEDCalibSample &sample,
                                     double circleSpacing) {

    std::vector<cv::Point3f> objectPoints;
    calib::LEDCalibPattern::getLEDObjectPoints(circleSpacing,
                                               objectPoints);

    Eigen::Matrix4d transformation;

    // compute the transformation matrix
    calib::LEDCalibPattern::makeTransformationMatrix(
                                              cam.getIntrisicMatrix(),
                                              cam.getDistortion(),
                                              sample.image_points,
                                              objectPoints,
                                              transformation);

    computeGlintOGLPoint(cam,
                         transformation,
                         objectPoints,
                         sample);
}


void GrSamples::computeGlintOGLPoint(const Camera &cam,
                                     const Eigen::Matrix4d &tm,
                                     const std::vector<cv::Point3f> &objectPoints,
                                     const calib::LEDCalibSample &sample) {

    // compute the normal
    Eigen::Vector3d eig_normal;
    calib::LEDCalibPattern::computeNormal(tm, eig_normal);

    const Eigen::Vector3d ocvVecInters = calib::computeIntersectionPoint(
                                                      sample.glint,
                                                      objectPoints,
                                                      eig_normal,
                                                      tm,
                                                      cam);

    glintOGLPoint[0] =  ocvVecInters(0);
    glintOGLPoint[1] = -ocvVecInters(1);
    glintOGLPoint[2] = -ocvVecInters(2);

}


void GrSamples::draw() {

    /*****************************************
     * Draw the camera
     *****************************************/

    drawCamera();


    /*****************************************
     * Draw the LED
     *****************************************/

    drawLED();



    /*****************************************
     * Draw pattern markers
     *****************************************/

    glPointSize(10);

    glBegin(GL_POINTS);

    for(int i = 0; i < 16; ++i) {

        glVertex3dv(patternOGLPoints + 3*i);

    }


    /*****************************************
     * Draw the glint
     *****************************************/

    // glint
    glVertex3dv(glintOGLPoint);


    glEnd();


    /*****************************************
     * Draw rays
     *****************************************/
    glBegin(GL_LINES);

    glVertex3d(0, 0, 0);
    glVertex3dv(vecIntersection);

    glVertex3dv(vecIntersection);

    double d = 70.0;
    double tmp[3] = {vecIntersection[0] + d * vecReflection[0],
                     vecIntersection[1] + d * vecReflection[1],
                     vecIntersection[2] + d * vecReflection[2]};

    glVertex3dv(tmp);

    glEnd();

}


void GrSamples::computeRays(const Camera &cam,
                            const calib::LEDCalibSample &sample,
                            double circleSpacing) {

    std::vector<cv::Point3f> objectPoints;
    calib::LEDCalibPattern::getLEDObjectPoints(circleSpacing,
                                               objectPoints);

    Eigen::Matrix4d transformation;

    // compute the transformation matrix
    calib::LEDCalibPattern::makeTransformationMatrix(
                                              cam.getIntrisicMatrix(),
                                              cam.getDistortion(),
                                              sample.image_points,
                                              objectPoints,
                                              transformation);

    computeRays(cam,
                transformation,
                objectPoints,
                sample);

}


void GrSamples::computeRays(const Camera &cam,
                            const Eigen::Matrix4d &tm,
                            std::vector<cv::Point3f> &objectPoints,
                            const calib::LEDCalibSample &sample) {

    // compute the normal
    Eigen::Vector3d eig_normal;
    calib::LEDCalibPattern::computeNormal(tm, eig_normal);

    const Eigen::Vector3d ocvVecInters = calib::computeIntersectionPoint(
                                                      sample.glint,
                                                      objectPoints,
                                                      eig_normal,
                                                      tm,
                                                      cam);

    vecIntersection[0] =  ocvVecInters(0);
    vecIntersection[1] = -ocvVecInters(1);
    vecIntersection[2] = -ocvVecInters(2);


    // and the reflection vector
    Eigen::Vector3d ocvVecRefl = calib::computeReflectionPoint(eig_normal,
                                                               ocvVecInters);

    vecReflection[0] =  ocvVecRefl(0);
    vecReflection[1] = -ocvVecRefl(1);
    vecReflection[2] = -ocvVecRefl(2);

}


// static
void GrSamples::drawLED(double x, double y, double z) {

	glPushMatrix();

    glTranslated(x, y, z);

		glColor4f(0, 0, 1, 1);

		glBegin(GL_QUADS);


			static const double sz2 = LED_SIDE_LENGTH / 2.0;

			// front face
			glVertex3d(-sz2,  sz2, sz2);
			glVertex3d(-sz2, -sz2, sz2);
			glVertex3d( sz2, -sz2, sz2);
			glVertex3d( sz2,  sz2, sz2);

			// left face
			glVertex3d(-sz2,  sz2, -sz2);
			glVertex3d(-sz2, -sz2, -sz2);
			glVertex3d(-sz2, -sz2, sz2);
			glVertex3d(-sz2,  sz2, sz2);

			// back face
			glVertex3d( sz2,  sz2, -sz2);
			glVertex3d( sz2, -sz2, -sz2);
			glVertex3d(-sz2, -sz2, -sz2);
			glVertex3d(-sz2,  sz2, -sz2);

			// right face
			glVertex3d(sz2,  sz2,  sz2);
			glVertex3d(sz2, -sz2,  sz2);
			glVertex3d(sz2, -sz2, -sz2);
			glVertex3d(sz2,  sz2, -sz2);

			// top face
			glVertex3d(-sz2, sz2, -sz2);
			glVertex3d(-sz2, sz2,  sz2);
			glVertex3d( sz2, sz2,  sz2);
			glVertex3d( sz2, sz2, -sz2);

			// bottom face
			glVertex3d(-sz2, -sz2,  sz2);
			glVertex3d(-sz2, -sz2, -sz2);
			glVertex3d( sz2, -sz2, -sz2);
			glVertex3d( sz2, -sz2,  sz2);

		glEnd();

	glPopMatrix();

}


void GrSamples::drawLED() {

    GrSamples::drawLED(posLED[0], posLED[1], posLED[2]);

}


void GrSamples::drawCamera() {

    static const double sz = 0.003;
    static const double sz2 = sz / 2;

    glColor4f(1, 0, 0, 1);

    glBegin(GL_QUADS);

    glVertex2d(-sz2,  sz2);
    glVertex2d(-sz2, -sz2);
    glVertex2d( sz2, -sz2);
    glVertex2d( sz2,  sz2);

    glEnd();

    glBegin(GL_TRIANGLES);

    // left side
    glVertex3d(-sz2,  sz2, 0.0);
    glVertex3d( 0.0,  0.0, sz);
    glVertex3d(-sz2, -sz2, 0.0);

    // bottom side
    glVertex3d(-sz2, -sz2, 0.0);
    glVertex3d( 0.0,  0.0, sz);
    glVertex3d( sz2, -sz2, 0.0);

    // right side
    glVertex3d( sz2, -sz2, 0.0);
    glVertex3d( 0.0,  0.0, sz);
    glVertex3d( sz2,  sz2, 0.0);

    // top side
    glVertex3d(-sz2,  sz2, 0.0);
    glVertex3d( 0.0,  0.0, sz);
    glVertex3d( sz2,  sz2, 0.0);

    glEnd();

}

