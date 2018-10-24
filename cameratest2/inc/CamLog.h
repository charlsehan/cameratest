#ifndef _CAMERA_INC_COMMON_CAMLOG_H_
#define _CAMERA_INC_COMMON_CAMLOG_H_


//#define LOG_TO_PRINTF

#ifdef LOG_TO_PRINTF
#include <cutils/log.h>
#define CAM_LOGV(fmt, arg...)       printf("V/ " fmt"\r\n", ##arg)
#define CAM_LOGD(fmt, arg...)       printf("D/ " fmt"\r\n", ##arg)
#define CAM_LOGI(fmt, arg...)       printf("I/ " fmt"\r\n", ##arg)
#define CAM_LOGW(fmt, arg...)       printf("W/ " fmt"\r\n", ##arg)
#define CAM_LOGE(fmt, arg...)       printf("E/ " fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)
#else
#include <cutils/log.h>
#define LOG_TAG "CAMERA_TEST"
#define CAM_LOGV(...)               ALOGV(__VA_ARGS__)
#define CAM_LOGD(...)               ALOGD(__VA_ARGS__)
#define CAM_LOGI(...)               ALOGI(__VA_ARGS__)
#define CAM_LOGW(...)               ALOGW(__VA_ARGS__)
#define CAM_LOGE(...)               ALOGE(__VA_ARGS__)
#endif

#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }


#endif  //_CAMERA_INC_COMMON_CAMLOG_H_

