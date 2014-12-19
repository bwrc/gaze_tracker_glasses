#include <opencv2/core/core.hpp>
#include <iostream>
#include <cstdio>
#include "CRTemplate.h"


/*****************************************************************************
 * CR templates
 ****************************************************************************/


namespace gt {


    void CRTemplate::makeTemplateImage(cv::Mat &retImg,
                                       int nW,
                                       int nRadius) {

        /*
         *	template size must be odd, because the centre element is not defined
         *	when even. Look at the ASCII image below:
         *
         *	| | |x| | |
         *
         */

        int maskLen = nW;
        if(maskLen % 2 == 0) {
            ++maskLen;
        }

        retImg.release();
        retImg = cv::Mat::zeros(maskLen, maskLen, CV_8UC1);

        // note that the centre is defined now because maskLen is odd
        int c = maskLen >> 1;
        int radius = nRadius;
        cv::circle(retImg, cv::Point(c, c), radius, cv::Scalar(255), CV_FILLED, CV_AA);

    }


    double CRTemplate::maskTests(const cv::Mat &__imgGrayCR,
                                 const cv::Point &point,
                                 const cv::Mat &__imgCrTemplate) {

        // coordinate
        const int x = point.x;
        const int y = point.y;

        /***********************************************************************
         * Define valid values for the boundaries
         ***********************************************************************/

        const int half_len = __imgCrTemplate.cols * 0.5;
        int xs = x - half_len;
        int xe = x + half_len;
        int ys = y - half_len;
        int ye = y + half_len;

        if(xs < 0 || xe >= __imgGrayCR.cols ||
           ys < 0 || ye >= __imgGrayCR.rows) {
            return std::numeric_limits<double>::max();
        }

        xs = xs < 0 ? 0 : xs;
        xe = xe >= __imgGrayCR.cols ? __imgGrayCR.cols - 1 : xe;
        ys = ys < 0 ? 0 : ys;
        ye = ye >= __imgGrayCR.rows ? __imgGrayCR.rows - 1 : ye;

        // the length and the height of the mask
        const int w = xe-xs + 1;
        const int h = ye-ys + 1;


        /***********************************************************************
         * Get the CR template
         ***********************************************************************/

        // zero if not restricted above
        const int diff_x = xs - (x-half_len);
        const int diff_y = ys - (y-half_len);

        // a header to the template. Is equal to the original if not restricted above
        const cv::Mat imgCrTemplate(__imgCrTemplate, cv::Rect(diff_x, diff_y, w, h));


        /***********************************************************************
         * Get the area in the image under rect
         ***********************************************************************/

        const cv::Mat imgCr(__imgGrayCR, cv::Rect(xs, ys, w, h));
        const unsigned char * const dataGray         = (unsigned char *)imgCr.data;
        const unsigned char * const dataCrTemplate   = (unsigned char *)imgCrTemplate.data;


        /***********************************************************************
         * Compute the maximum possible error. The maximum error is the sum
         * of maximum per pixel errors. The per pixel errors are scaled between
         * [0, 1].
         ***********************************************************************/
        double dMaxError = w*h;

        double dSumErr = 0.0;

        const int stepCrTemplate = imgCrTemplate.step;
        const int stepGray       = imgCr.step;

        // compute the sum of squared absolute differences
        for(int row = 0; row < h; ++row) {

            int indCrTemplate = row * stepCrTemplate;
            int indGray	      = row * stepGray;

            // not inclusive
            int indCREnd = indCrTemplate + w;

            while(indCrTemplate < indCREnd) {

                double val1 = dataCrTemplate[indCrTemplate++];
                double val2 = dataGray[indGray++];

                double dDiff = (val1 - val2) / 255.0;
                dSumErr += dDiff*dDiff;

            }

        }

        double ret = dSumErr / dMaxError;

        return ret;

    }


    double CRTemplate::testCircularity(const cv::Mat &imgBinary,
                                       const cv::Point &com) {

        int nW = imgBinary.cols;
        int nH = imgBinary.rows;
        int nStep = imgBinary.step;
        unsigned char *pData = (unsigned char *)imgBinary.data;

        // compute the how much we can go to the left
        int nLeft = 0;
        for(int i = com.x-1; i >= 0; --i) {

            if(pData[com.y*nStep + i] == 0) {
                break;
            }

            ++nLeft;
        }

        // compute the how much we can go to the righ
        int nRight = 0;
        for(int i = com.x+1; i < nW; ++i) {

            if(pData[com.y*nStep + i] == 0) {
                break;
            }

            ++nRight;
        }

        // the width of the Cr
        int nHor = nLeft + nRight + 1;



        // compute the how much we can go up
        int nUp = 0;
        for(int i = com.y-1; i >= 0; --i) {

            if(pData[i*nStep + com.x] == 0) {
                break;
            }

            ++nUp;
        }

        // compute the how much we can go down
        int nDown = 0;
        for(int i = com.y+1; i < nH; ++i) {

            if(pData[i*nStep + com.x] == 0) {
                break;
            }

            ++nDown;
        }

        // the height of the Cr
        int nVer = nUp + nDown + 1;

        return 1.0 - (nHor > nVer ? (double)nVer / (double)nHor : (double)nHor / (double)nVer);

    }



    /*****************************************************************************
     * Class for storing multiple CRs
     ****************************************************************************/

    CRs::CRs()
    {
        max_nof = 0;
    }

    void CRs::setMax(int _max)
    {
        max_nof = _max;
    }

    std::vector<cv::Point2d>& CRs::getCentres()
    {
        return centres;
    }

    const std::vector<cv::Point2d>& CRs::getCentres() const
    {
        return centres;
    }

    int CRs::getMax()
    {
        return max_nof;
    }



} // end of "namespace gt"

