#ifndef _CAMERA_TEST_UTILS_H_
#define _CAMERA_TEST_UTILS_H_

#include "vtouch/VirtualTouch.h"

bool
saveBufToFile(
    char const*const fname,
    uint8_t*const buf,
    uint32_t const size
);


void reportMockPoint(int i, int fd);
bool writeTouchPoints(VirtualTouch* vtouch, unsigned char* pixelPtr, int fd);

#endif  //_CAMERA_TEST_UTILS_H_

