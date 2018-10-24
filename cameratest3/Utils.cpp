#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "inc/CamLog.h"
#include "inc/Utils.h"

/******************************************************************************
* save the buffer to the file
*******************************************************************************/
bool
saveBufToFile(char const*const fname, uint8_t *const buf, uint32_t const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("opening file [%s]", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes", size, fname, cnt);
    ::close(fd);
    return true;
}

void reportMockPoint(int i, int fd) {
    if (i % 10 == 0 && i % 100 >= 10 && i % 100 <= 90) {
        MultiTouchPoint mpt;
        memset(&mpt, 0, sizeof(mpt));

        for(int i = 0; i < MAX_POINT; i++) {
            mpt.points[i].abs_tracking_id = INVALID_VALUE;
            mpt.points[i].abs_position_x = INVALID_VALUE;
            mpt.points[i].abs_position_y = INVALID_VALUE;
        }

        if (i % 100 == 10) {
            mpt.btn_touch = BTN_TOUCH_DOWN;
        } else if (i % 100 == 90) {
            mpt.btn_touch = BTN_TOUCH_UP;
        } else {
            mpt.btn_touch = INVALID_VALUE;
        }
        mpt.abs_x = i;
        mpt.abs_y = i;

        mpt.points[0].abs_tracking_id = 1;
        mpt.points[0].abs_position_x = mpt.abs_x;
        mpt.points[0].abs_position_y = mpt.abs_y;

        int ret = ::write(fd, &mpt, sizeof(MultiTouchPoint));
        MY_LOGD("point: x %d, y %d, action %d, ret %d", mpt.abs_x, mpt.abs_y, mpt.btn_touch, ret);
    }
}


bool writeTouchPoints(VirtualTouch* vtouch, unsigned char* pixelPtr, int fd) {

    if(vtouch == NULL) {
        return false;
    }


    MultiTouchPoint mpt;
    memset(&mpt, 0, sizeof(mpt));

    bool ret = vtouch->handleImage(pixelPtr, mpt);

    if(!ret) return false;

    MY_LOGI("--------btn_touch = %d, abs_x = %d, abs_y = %d", mpt.btn_touch, mpt.abs_x, mpt.abs_y);
    if(mpt.btn_touch != BTN_TOUCH_UP) {
        for (int i = 0; i < MAX_POINT; i++) {
            if (mpt.points[i].abs_position_x != INVALID_VALUE ||
                mpt.points[i].abs_position_y != INVALID_VALUE ||
                mpt.points[i].abs_tracking_id != INVALID_VALUE ||
                    mpt.points[i].abs_slot != INVALID_VALUE) {
                MY_LOGI("    ----slot(%d) : slot_id = %d, tracking_id = %d, abs_position_x = %d, abs_position_y = %d, ",
                     i, mpt.points[i].abs_slot, mpt.points[i].abs_tracking_id, mpt.points[i].abs_position_x,
                     mpt.points[i].abs_position_y);
            }
        }
    }

    int count = ::write(fd, &mpt, sizeof(MultiTouchPoint));

    return count == sizeof(MultiTouchPoint);
}
