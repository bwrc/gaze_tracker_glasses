#include "GLCornea.h"
#include <GL/gl.h>
#include <stdio.h>

static double dPi = 3.14159265359;


namespace gui {


    static void drawSphere(const cv::Point3d &c,
                    double r,int n,
                    double theta1, double theta2,
                    double phi1, double phi2);


    static void computeCameraProjection(const cv::Mat &matIntr,
                                        int w,
                                        int h,
                                        double pMat[16]);

    static void drawCircle(const cv::Point3d &pos, double dRadius, int n);

    static void drawCornerBall(const cv::Point3d &sphereCentre, double dRadius);


    GLCornea::GLCornea(const View &v) : GLWidget(v) {

        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);


        GLfloat dAlphaLight = 0.2;
        GLfloat ambientGlobal[4]  = {0.5, 0.5, 0.5, dAlphaLight}; // default 0.2, 0.2, 0.2, 1.0};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientGlobal);


        // create shader
        m_pShader = new Shader();
        m_pShader->create("shaders/perpixel.vert",
                          "shaders/perpixel.frag");


        // material settings
        GLfloat dAlphaMaterial = 0.15;
        GLfloat dShininnes = 127.0f;
        GLfloat materialDiffuse[]  = {1.0, 0.0, 0.0, dAlphaMaterial};
        GLfloat materialSpecular[] = {1.0, 1.0, 1.0, 4*dAlphaMaterial};
        GLfloat materialEmission[] = {0.0, 0.0, 0.0, dAlphaMaterial};

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &dShininnes);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission);

    }


    void GLCornea::setLightPositions(const std::vector<cv::Point3d> &vecLightPos) {

        m_vecLightPos = vecLightPos;
        setLightPositions();

    }


    void GLCornea::setLightPositions() {

        for(size_t i = 0; i < m_vecLightPos.size(); ++i) {

            const cv::Point3d &light = m_vecLightPos[i];

            // light settings
            GLfloat dAlphaLight = 0.81;
            GLfloat lightSpecular[] = {1.0, 1.0, 1.0, dAlphaLight};
            GLfloat lightAmbient[]  = {0.1, 0.1, 0.1, dAlphaLight};
            GLfloat lightDiffuse[]  = {1.0, 1.0, 1.0, dAlphaLight};

            GLfloat lightPos[]       = {(GLfloat)light.x,
                                        (GLfloat)light.y,
                                        (GLfloat)light.z,
                                        1.0f}; // w=1, point light

            GLenum enLight = GL_LIGHT0 + i;
            glEnable(enLight);
            glLightfv(enLight, GL_SPECULAR, lightSpecular);
            glLightfv(enLight, GL_AMBIENT, lightAmbient);
            glLightfv(enLight, GL_DIFFUSE, lightDiffuse);
            glLightfv(enLight, GL_POSITION, lightPos);

            // set attenuation
            glLightf(enLight, GL_CONSTANT_ATTENUATION, 4.6);
            glLightf(enLight, GL_LINEAR_ATTENUATION, 1.0);
            glLightf(enLight, GL_QUADRATIC_ATTENUATION, 0.5);

        }

    }


    void GLCornea::render(const ResultData *res,
                          const cv::Mat &matIntr,
                          const cv::Mat &matDist,
                          double dRho, double dRd) {

        glViewport(view.x, view.y, view.w, view.h);

        /***************************************************************
         * Draw the cornea sphere
         ***************************************************************/
        m_pShader->use();
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glClear(GL_DEPTH_BUFFER_BIT); // because the texture is on the surface
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        // compute the projection matrix from the intrinsic parameters
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        double mat[16];

        computeCameraProjection(matIntr, view.w, view.h, mat);
        glLoadMatrixd(mat);

        // set model view for drawing
        glMatrixMode(GL_MODELVIEW);


        cv::Point3d corneaCentre;

        GLdouble pdColorPupil[4] = {0.0, 0.0, 0.0, 0.2};
        GLdouble pdColorCornea[4];

        if(res->bTrackSuccessfull) {

            // convert from OpenCV to OpenGL
            corneaCentre = res->corneaCentre;
            corneaCentre.y *= -1;
            corneaCentre.z *= -1;

            pdColorCornea[0] = 0.7;
            pdColorCornea[1] = 0.3;
            pdColorCornea[2] = 0.3;
            pdColorCornea[3] = 0.01;

            m_lastValidCorneaCentre = corneaCentre;

        }
        else {

            // track not ok, use last valid
            corneaCentre = m_lastValidCorneaCentre;

            pdColorCornea[0] = 0.3;
            pdColorCornea[1] = 0.3;
            pdColorCornea[2] = 0.7;
            pdColorCornea[3] = 0.01;

        }

        // -y and -z because OpenCV to OpenGL
        cv::Point3d pupilCentre = res->pupilCentre;
        pupilCentre.y *= -1;
        pupilCentre.z *= -1;

        // gaze vector
        double pVecGaze[3] = {pupilCentre.x - corneaCentre.x,
                              pupilCentre.y - corneaCentre.y,
                              pupilCentre.z - corneaCentre.z};

        // normalise
        double dLenInv = 1.0 / sqrt(pVecGaze[0]*pVecGaze[0] +
                                    pVecGaze[1]*pVecGaze[1] +
                                    pVecGaze[2]*pVecGaze[2]);

        /*
         * dTheta = acos(vec1 * vec2 / |vec1| * |vec2|)
         *
         * Compute the angle between the gaze vector and (0, 0, 1)
         */
        double dThetaRad = acos(pVecGaze[2] * dLenInv);
        double dThetaDeg = dThetaRad * 180.0 / dPi;


        /*
         * Compute the rotation vector (0, 0, 1) x pGazeVec
         *
         * u x v = (uy*vz - uz*vy)i + (uz*vx - ux*vz)j + (ux*vy - uy*vx)k
         */
        double pdRot[3] = {0.0 - 1.0*pVecGaze[1],
                           1.0*pVecGaze[0],
                           0.0};

        dLenInv = 1.0 / sqrt(pdRot[0]*pdRot[0] +
                             pdRot[1]*pdRot[1] +
                             pdRot[2]*pdRot[2]);

        pdRot[0] *= dLenInv;
        pdRot[1] *= dLenInv;
        pdRot[2] *= dLenInv;


        glPushMatrix(); {

            glLoadIdentity();

            glTranslated(corneaCentre.x, corneaCentre.y, corneaCentre.z);
            glRotated(dThetaDeg, pdRot[0], pdRot[1], pdRot[2]);

            const double dRadius = 0.002;//pGazeTracker->getPupilRadius();

            glColor4dv(pdColorPupil);
            drawCircle(cv::Point3d(0, 0, dRd), dRadius, 60);

            glColor4dv(pdColorCornea);

            drawCornerBall(cv::Point3d(), dRho);


        } glPopMatrix();


        glDisable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        glColor4d(1, 1, 1, 1);

        glUseProgram(0);

    }


    void drawCornerBall(const cv::Point3d &sphereCentre, double dRadius) {

        // in radians 0 < theta < 2pi, -pi/2 < phi < pi/2
        double dTheta1 =  0.0;
        double dTheta2 =  2*dPi/2.0;
        double dPhi1   = -dPi / 2.0;
        double dPhi2   =  dPi / 2.0;

        drawSphere(sphereCentre,
                   dRadius,
                   60,
                   dTheta1, dTheta2,
                   dPhi1, dPhi2);

    }


    void drawCircle(const cv::Point3d &pos, double dRadius, int n) {

        glBegin(GL_TRIANGLE_FAN); {

            glNormal3d(0.0, 0.0, 1.0);
            glVertex3d(pos.x, pos.y, pos.z);

            double dAng = 0.0;
            double dDAng = 2*dPi / (double)(n - 1);

            for(int i = 0; i < n; ++i) {

                double dX = dRadius * cos(dAng);
                double dY = dRadius * sin(dAng);

                glVertex3d(pos.x + dX, pos.y + dY, pos.z);

                dAng += dDAng;

            }

        } glEnd();

    }


    // http://strawlab.org/2011/11/05/augmented-reality-with-OpenGL/
    void computeCameraProjection(const cv::Mat &matIntr,
                                 int w,
                                 int h,
                                 double pMat[16]) {

        const double znear = 0.001;
        const double zfar  = 0.1;
        const double x0    = 0.0;
        const double y0    = 0.0;

        const double K00 = matIntr.at<double>(0, 0);
        const double K01 = matIntr.at<double>(0, 1);
        const double K02 = matIntr.at<double>(0, 2);
        const double K11 = matIntr.at<double>(1, 1);
        const double K12 = matIntr.at<double>(1, 2);

        // Row 1
        pMat[0]  = 2.0*K00 / w;
        pMat[4]  = -2.0*K01 / h;
        pMat[8]  = (w - 2.0*K02 + 2.0*x0) / w;
        pMat[12] = 0;

        // Row 2
        pMat[1]  = 0.0;
        pMat[5]  = 2.0*K11 / h;
        pMat[9]  = (-h + 2.0*K12 + 2.0*y0) / (double)h;
        pMat[13] = 0.0;

        // Row 3
        pMat[2]  = 0.0;
        pMat[6]  = 0.0;
        pMat[10] = (-zfar-znear) / (zfar - znear);
        pMat[14] = -2.0*zfar*znear / (zfar-znear);

        // Row 4
        pMat[3]  = 0.0;
        pMat[7]  = 0.0;
        pMat[11] = -1.0;
        pMat[15] = 0.0;

    }


    /*
     *	Create a sphere centered at c, with radius r, and precision n
     *	Draw a point for zero radius spheres
     *	Use CCW facet ordering
     *	Partial spheres can be created using theta1->theta2, phi1->phi2
     *	in radians 0 < theta < 2pi, -pi/2 < phi < pi/2
     *
     *	http://local.wasp.uwa.edu.au/~pbourke/texture_colour/texturemap/sphere.c
     */
    void drawSphere(const cv::Point3d &c,
                    double r,int n,
                    double theta1, double theta2,
                    double phi1, double phi2) {

        int i,j;
        double j1divn,dosdivn,unodivn=1/(double)n,ndiv2=(double)n/2,t1,t2,t3,cost1,cost2,cte1,cte3;
        cte3 = (theta2-theta1)/n;
        cte1 = (phi2-phi1)/ndiv2;
        dosdivn = 2*unodivn;
        cv::Point3d e,p,e2,p2;

        /* Handle special cases */
        if(r < 0)
            r = -r;
        if(n < 0){
            n = -n;
            ndiv2 = -ndiv2;
        }
        if(n < 4 || r <= 0) {
            glBegin(GL_POINTS);
            glVertex3f(c.x,c.y,c.z);
            glEnd();
            return;
        }

        t2=phi1;
        cost2=cos(phi1);
        j1divn=0;
        for(j = 0; j < ndiv2; ++j) {
            t1		= t2;//t1 = phi1 + j * cte1;
            t2		+= cte1;//t2 = phi1 + (j + 1) * cte1;
            t3		= theta1 - cte3;
            cost1	= cost2;//cost1=cos(t1);
            cost2	= cos(t2);
            e.y		= sin(t1);
            e2.y	= sin(t2);
            p.y		= c.y + r * e.y;
            p2.y	= c.y + r * e2.y;


            glBegin(GL_QUAD_STRIP);

			j1divn+=dosdivn;//=2*(j+1)/(double)n;

			for(i = 0; i <= n; ++i) {

				t3 += cte3;
				e.x = cost1 * cos(t3);
				e.z = cost1 * sin(t3);
				p.x = c.x + r * e.x;
				p.z = c.z + r * e.z;
				glNormal3f(e.x,e.y,e.z);
				glVertex3f(p.x,p.y,p.z);

				e2.x = cost2 * cos(t3);
				e2.z = cost2 * sin(t3);
				p2.x = c.x + r * e2.x;

				p2.z = c.z + r * e2.z;
				glNormal3f(e2.x,e2.y,e2.z);
				glVertex3f(p2.x,p2.y,p2.z);

			}

            glEnd();
        }

    }



} // end of "namespace gui"

