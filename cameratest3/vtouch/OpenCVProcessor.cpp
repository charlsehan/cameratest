//
// Created by yangqingsu0105 on 2018/9/14.
//

#include "OpenCVProcessor.h"
#include <android/log.h>

#define  LOG_TAG    "OpenCVProcessor"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
using namespace cv;
using namespace std;



int OpenCVProcessor::findTouchPoint(unsigned char *inputImage) {

    Mat temp(mImageHeight, mImageWidth, CV_8UC1, inputImage);
    temp(mActiveArea).copyTo(mImageActive);
    threshold(mImageActive, mImageThreshold, BIN_THRESHOLD, 255, THRESH_BINARY);
    morphologyEx(mImageThreshold, mImageMorphology, MORPH_OPEN, mMatKernel);
    dilate(mImageMorphology, mImageDilate, mMatKernel);

    // find connected components
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( mImageDilate, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );


    int pointIndex = 0;
    int size = MIN(MAX_BUFFER_SIZE, contours.size());
    if(size > 0) {
        for (int i = 0; i < size; i++) {
            Moments mu = moments(contours[i], false);
            Point2f mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
            float area = contourArea(contours[i]);

            if (area < mMaxArea && area > mMinArea) {
                mBuffer[pointIndex].x = mc.x + mActiveArea.x;
                mBuffer[pointIndex++].y = mc.y + mActiveArea.y;
                LOGI("*********size(%d,%d),x=(%d+%f),y=(%d+%f),area(%f,%f):%f",mImageDilate.cols, mImageDilate.rows,
                     mActiveArea.x, mc.x, mActiveArea.y, mc.y, mMinArea, mMaxArea, area);
            } else {
                LOGI("invalid centers:%f, %f, area: %f",mActiveArea.x + mc.x, mActiveArea.x + mc.y, area);
            }
        }
    }

    return pointIndex;
}
