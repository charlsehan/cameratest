//
// Created by yangqingsu0105 on 2018/9/14.
//

#ifndef OPENCVROCESSOR_H
#define OPENCVROCESSOR_H

#include "ImageProcessor.h"


class OpenCVProcessor: public ImageProcessor
{
public:
    OpenCVProcessor():ImageProcessor(), mMatKernel(Size(3,3), CV_32FC1, Scalar(1)){};
    int findTouchPoint(unsigned char *inputImage);

private:
    Mat mImageActive;
    Mat mImageMorphology, mImageDilate, mImageThreshold;

    Mat mMatKernel;
};

#endif