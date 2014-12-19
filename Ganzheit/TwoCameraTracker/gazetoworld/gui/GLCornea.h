#ifndef GL_CORNEA_H
#define GL_CORNEA_H


#include "Shader.h"
#include "ResultData.h"
#include "GLWidget.h"
#include <vector>




namespace gui {

    class GLCornea : public GLWidget {

    public:

        GLCornea(const View &v);

        void render(const ResultData *res,
                    const cv::Mat &matIntr,
                    const cv::Mat &matDist,
                    double dRho, double dRd);

        void setLightPositions(const std::vector<cv::Point3d> &vecLightPos);

    private:

        void setLightPositions();

        cv::Point3d m_lastValidCorneaCentre;

        std::vector<cv::Point3d> m_vecLightPos;

        Shader *m_pShader;

    };


} // end of "namespace gui"


#endif
