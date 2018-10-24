//
// Created by richard on 18-9-20.
//

#ifndef DRAWTEST_VIRTUALTOUCH_H
#define DRAWTEST_VIRTUALTOUCH_H

#include "ImageProcessor.h"

#define MAX_POINT 10
#define INVALID_VALUE -10

enum BTN_EVENT {BTN_TOUCH_UP = 0, BTN_TOUCH_DOWN = 1};

struct MultiTouchPoint {
    struct {
        int  	abs_position_x;
        int   	abs_position_y;
        int     abs_tracking_id;
        int     abs_slot;
    } points[MAX_POINT];
    int  	abs_x;
    int   	abs_y;
    int     btn_touch;
};

class VirtualTouch {
public:
    VirtualTouch();
    ~VirtualTouch();

    bool init(const char* path, int screenWidth, int screenHeight, int imageWidth, int imageHeight);

    bool handleImage(unsigned char *image, MultiTouchPoint& mpt);
private:

    void clear();
    int findNerestSlot(Point &point, int& space);
    int findEmptySlot();

    Point wrapPoint(Point2f pt);

    ImageProcessor* mImageProcessor;

    double mCalibMatrix[3][3];

    int mTrackingId;
    Point mLastPoint[MAX_POINT];
    bool mDownState;

    int mScreenWidth;
    int mScreenHeight;
    int mMaxSpace;
    MultiTouchPoint mInvalidatePoint;
};

#endif //DRAWTEST_VIRTUALTOUCH_H
