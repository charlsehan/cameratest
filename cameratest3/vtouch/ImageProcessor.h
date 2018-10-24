//
// Created by yangqingsu0105 on 2018/9/14.
//

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <opencv2/opencv.hpp>

#define BIN_THRESHOLD 70       // threshold for binarilizing gray images


using namespace cv;

class ImageProcessor
{
protected:
    static const int MAX_BUFFER_SIZE = 20;
public:
    ImageProcessor(){}
    void init(int width, int height, Rect rect);
    virtual int findTouchPoint(unsigned char *inputImage) = 0;
    Point2f* getPointsBufferPtr(){return mBuffer;};

protected:
    Point2f mBuffer[MAX_BUFFER_SIZE];
    int mImageWidth;
    int mImageHeight;

    float mMinArea;
    float mMaxArea;
    Rect mActiveArea;

};

#endif