#ifndef CR_TEMPLATE_H
#define CR_TEMPLATE_H

#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


namespace gt {


    /*****************************************************************************
     * CR template declarations
     ****************************************************************************/

    class CRTemplate {

    public:

        /*
         * Makes a template image using the given parameters:
         * nW      : width and height of the template image, if not odd, will be incremented by 1.
         * nRadius : radius of the cr
         */
        static void makeTemplateImage(cv::Mat &retImg,
                                      int nW,
                                      int nRadius);

        /*
         * Test the candidate pointed by point with the given mask.
         * Tests how much squared error there is per pixel. The error
         * is scaled between [0, 1].
         */
        static double maskTests(const cv::Mat &imgGrayCR,
                                const cv::Point &point,
                                const cv::Mat &imgMask);

        /*
         * Test the circularity of the CR. This is tested by simply
         * computing the ratio between the CR width and height.
         * returns a value between [0, 1]. The image must be a binary
         * image where the CR is 255.
         */
        static double testCircularity(const cv::Mat &imgBinary,
                                      const cv::Point &com);

    private:

        /*
         * This class provides only static functions, instantiation of this
         * class is not allowed.
         */
        CRTemplate();
        CRTemplate(const CRTemplate &);
        CRTemplate &operator=(const CRTemplate &);

    };



    /*****************************************************************************
     * Class declaration for storing multiple CRs
     ****************************************************************************/

    class CRs {
    public:
        CRs();
        void setMax(int _max);
        std::vector<cv::Point2d> &getCentres();
        const std::vector<cv::Point2d> &getCentres() const;
        int getMax();

    private:
        int max_nof;
        std::vector<cv::Point2d> centres;
    };


} // end of "namespace gt"


#endif // CR_TEMPLATE_H

