//
// Created by richard on 18-9-20.
//

#include "VirtualTouch.h"
//#include "RawProcessor.h"
#include "OpenCVProcessor.h"
#include "VirtualTouch.h"
#include <android/log.h>

#define  LOG_TAG    "VirtualTouch"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


#define EXTRA_SPACE 10
#define POINT_MIN_SPACE 30

VirtualTouch::VirtualTouch() {
    mImageProcessor = new /*RawProcessor();*/OpenCVProcessor();
    mTrackingId = 1;
    mDownState = false;
    mInvalidatePoint.btn_touch = INVALID_VALUE;
    mInvalidatePoint.abs_x = INVALID_VALUE;
    mInvalidatePoint.abs_y = INVALID_VALUE;
    for(int i = 0; i < MAX_POINT; i++) {
        mInvalidatePoint.points[i].abs_tracking_id = INVALID_VALUE;
        mInvalidatePoint.points[i].abs_slot = INVALID_VALUE;
        mInvalidatePoint.points[i].abs_position_x = INVALID_VALUE;
        mInvalidatePoint.points[i].abs_position_y = INVALID_VALUE;
    }
}

VirtualTouch::~VirtualTouch() {
    delete mImageProcessor;
}

bool VirtualTouch::init(const char* path, int screenWidth, int screenHeight, int imageWidth, int imageHeight) {
    FILE *file = fopen(path, "r");
    if(file) {
        Rect rect;
        fscanf(file, "%lf,%lf,%lf,",&mCalibMatrix[0][0], &mCalibMatrix[0][1], &mCalibMatrix[0][2]);
        fscanf(file, "%lf,%lf,%lf,",&mCalibMatrix[1][0], &mCalibMatrix[1][1], &mCalibMatrix[1][2]);
        fscanf(file, "%lf,%lf,%lf,",&mCalibMatrix[2][0], &mCalibMatrix[2][1], &mCalibMatrix[2][2]);
        fscanf(file, "%d,%d,%d,%d,",&rect.x, &rect.y, &rect.width, &rect.height);
        fclose(file);

        mScreenWidth = screenWidth;
        mScreenHeight = screenHeight;
        mMaxSpace = mScreenWidth*mScreenWidth+mScreenHeight*mScreenHeight;
        mImageProcessor->init(imageWidth, imageHeight, rect);
        clear();
        return true;
    }
    return false;
}

void VirtualTouch::clear() {
    for(int i = 0; i < MAX_POINT; i++) {
        mLastPoint[i].x = INVALID_VALUE;
        mLastPoint[i].y = INVALID_VALUE;
    }
}


int VirtualTouch::findNerestSlot(Point &point, int& space) {
    space = mMaxSpace;
    int index = 0;

    for(int i = 0; i < MAX_POINT; i++) {
        if(mLastPoint[i].x != INVALID_VALUE && mLastPoint[i].y != INVALID_VALUE) {
           int x = abs(mLastPoint[i].x-point.x);
           int y = abs(mLastPoint[i].y-point.y);
           int k = x*x + y*y;
           if(k < space) {
               space = k;
               index = i;
           }
        }
    }

    space = sqrt(space);
    return index;
}

int VirtualTouch::findEmptySlot() {
    for(int i = 0; i < MAX_POINT; i++) {
        if(mLastPoint[i].x == INVALID_VALUE && mLastPoint[i].y == INVALID_VALUE) {
            return i;
        }
    }
    return -1;
}




Point VirtualTouch::wrapPoint(Point2f src) {
    Point dst;
    double factor = mCalibMatrix[2][0] * src.x + mCalibMatrix[2][1] * src.y + mCalibMatrix[2][2];
    dst.x = (mCalibMatrix[0][0] * src.x + mCalibMatrix[0][1] * src.y + mCalibMatrix[0][2]) / factor;
    dst.y = (mCalibMatrix[1][0] * src.x + mCalibMatrix[1][1] * src.y + mCalibMatrix[1][2]) / factor;
    return dst;
}


bool VirtualTouch::handleImage(unsigned char *image, MultiTouchPoint& mpt) {

    memcpy(&mpt, &mInvalidatePoint, sizeof(mpt));
    int count = mImageProcessor->findTouchPoint(image);
    count = MIN(count, MAX_POINT);

    if(count == 0) {
        if(mDownState) {
            for(int i = 0; i < MAX_POINT; i++) {
                if (mLastPoint[i].x > 0 && mLastPoint[i].y > 0) {
                    mpt.points[i].abs_tracking_id = -1;
                    mpt.points[i].abs_slot = i;
                }
            }
            mDownState = false;
            mTrackingId++;
            clear();
            mpt.btn_touch = BTN_TOUCH_UP;
            return true;
        } else {
            return false;
        }
    }


    Point2f* buffer = mImageProcessor->getPointsBufferPtr();


    bool flags[MAX_POINT];
    memset(flags, 0, sizeof(flags)*sizeof(bool));


    for (int i = 0; i < count; i++) {
        Point pt = wrapPoint(buffer[i]);
        LOGI("screen(%d,%d), count = %d, x = %d, y = %d", mScreenWidth, mScreenHeight, count, pt.x, pt.y);
        if(pt.x > -EXTRA_SPACE && pt.x < (mScreenWidth + EXTRA_SPACE) &&
                pt.y > -EXTRA_SPACE && pt.y < (mScreenHeight + EXTRA_SPACE)) {
            int space;
            int id = findNerestSlot(pt, space);
            if(space > POINT_MIN_SPACE) {
                int index = findEmptySlot();
                if(index >= 0 && index < MAX_POINT) {
                    id = index;
                    mpt.points[id].abs_tracking_id = mTrackingId++;
                }
            }
            mpt.points[id].abs_slot = id;
            mpt.points[id].abs_position_x = pt.x;
            mpt.points[id].abs_position_y = pt.y;

            mLastPoint[id].x = pt.x;
            mLastPoint[id].y = pt.y;
            flags[id] = true;

            if(!mDownState) {
                mDownState = true;
                mpt.btn_touch = BTN_TOUCH_DOWN;
            }
        }
    }

    if(!mDownState)
        return false;

    for (int i = 0; i < MAX_POINT; i++) {
        if (flags[i] == false && mLastPoint[i].x != INVALID_VALUE && mLastPoint[i].y != INVALID_VALUE) {
            mpt.points[i].abs_tracking_id = -1;
            mpt.points[i].abs_slot = i;
            mLastPoint[i].x = INVALID_VALUE;
            mLastPoint[i].y = INVALID_VALUE;
        }
    }

    for (int i = 0; i < MAX_POINT; i++) {
        if (flags[i]) {
            mpt.abs_x = mpt.points[i].abs_position_x;
            mpt.abs_y = mpt.points[i].abs_position_y;
            break;
        }
    }

    return true;
}
