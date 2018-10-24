//
// Created by yangqingsu0105 on 2018/9/14.
//

#include "ImageProcessor.h"


void ImageProcessor::init(int width, int height, Rect rect)
{
    mImageWidth = width;
    mImageHeight = height;
    mActiveArea = rect;

    mMaxArea = mImageWidth * mImageHeight * 0.072 * 0.072;
    mMinArea = mImageWidth * mImageHeight * 0.0039 * 0.0039;
};