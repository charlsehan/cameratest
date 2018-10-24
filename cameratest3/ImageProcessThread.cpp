#include <utils/Log.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <cutils/memory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <fcntl.h>
#include <sys/prctl.h>

#include <Camera.h>
#include <CameraParameters.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>

#include <system/window.h>
#include <system/camera.h>
#include <hardware/camera.h>

#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/ImageProcessThread.h"

#include "vtouch/VirtualTouch.h"


using namespace android;


int ImageProcessThread::mPointsFd = NULL;
VirtualTouch* ImageProcessThread::mVirtualTouch = NULL;

char const CALIBRATION_PATH[] = "/sdcard/calibration.dat";


ImageProcessThread::ImageProcessThread()
{
    mArgument = new Argument;
    mArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
    mArgument->mi4CamMode = MtkCameraParameters::CAMERA_MODE_MTK_PRV;
    mArgument->mOpenId = 0;
    mArgument->mPreviewSize = android::Size(640, 480);
    mArgument->mDisplayOrientation = 0;
    mArgument->mDisplayOn = true;
    mArgument->ms8PrvFmt = String8(CameraParameters::PIXEL_FORMAT_YUV420P);
}


ImageProcessThread::~ImageProcessThread()
{
}


bool ImageProcessThread::start()
{
    if  ( 1 == ::android_atomic_release_load(&mThreadRunning) ) {
        MY_LOGW("the test has been starting before...skip this command.");
        return  false;
    }

    reset();
    ::android_atomic_release_store(1, &mThreadRunning);
    return  (OK == run("ImageProcessThread::start"));
}


bool ImageProcessThread::stop()
{
    requestExit();
    {
        android::Mutex::Autolock _lock(mThreadMutex);
        ::android_atomic_release_store(0, &mThreadRunning);
        mThreadCond.broadcast();
    }

    return  (OK == join());
}


bool ImageProcessThread::reset()
{
    ::android_atomic_release_store(0, &mThreadRunning);

    if (mPointsFd != NULL) {
        ::close(mPointsFd);
    }

    if (mVirtualTouch != NULL) {
        delete mVirtualTouch;
        mVirtualTouch = NULL;
    }

    return  true;
}


// Good place to do one-time initializations
status_t ImageProcessThread::readyToRun()
{
    ::prctl(PR_SET_NAME,"ImageProcessThread", 0, 0, 0);

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_OTHER;
    int const priority  = 0;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);

    //  get
    ::sched_getparam(0, &sched_p);

    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    return NO_ERROR;
}


bool ImageProcessThread::threadLoop()
{
    status_t status;
    bool ret = false;

    ret =
            connectCamera(mArgument->mOpenId)
        &&  setupParameters()
            ;
    if  ( ! ret )
    {
        goto lbExit;
    }

    // 1. Get ready of touch input device, we will write touch points to that device
    //mPointsFd = ::open("/sys/bus/platform/drivers/asu-fan/asu-fan/pointset", O_WRONLY | O_NDELAY);
    //if(mPointsFd < 0) {
    //    MY_LOGE("open touch input device file error, mPointsFd=%d", mPointsFd);
    //    goto lbExit;
    //}

    // 2. Init an VirtualTouch, input image to it, we got touch points
    mVirtualTouch = new VirtualTouch();

    if(!mVirtualTouch->init(CALIBRATION_PATH, 1280, 720,
                            mArgument->mPreviewSize.width, mArgument->mPreviewSize.height)) {
        MY_LOGE("init virtual touch error");
        goto lbExit;
    }

    // 3. Start camera preview
    if (mDevice->ops->enable_msg_type)
        mDevice->ops->enable_msg_type(mDevice, CAMERA_MSG_PREVIEW_FRAME);

    MY_LOGD("after enable_msg_type: %d\n", CAMERA_MSG_PREVIEW_FRAME);

    if (mDevice->ops->start_preview)
        mDevice->ops->start_preview(mDevice);

    {
        android::Mutex::Autolock _lock(mThreadMutex);
        while   ( 1 == ::android_atomic_release_load(&mThreadRunning) )
        {
            nsecs_t nsTimeoutToWait = 30LL*1000LL*1000LL*1000LL;//wait 30 sec.
            MY_LOGD("Start to wait %lld sec...", nsTimeoutToWait/1000000000LL);
            status_t status = mThreadCond.waitRelative(mThreadMutex, nsTimeoutToWait);
        }
    }

    if (mDevice->ops->stop_preview)
        mDevice->ops->stop_preview(mDevice);

lbExit:
    disconnectCamera();
    reset();
    return  false;
}


bool ImageProcessThread::connectCamera(int id)
{
    int ret = OK;

    int err = hw_get_module(CAMERA_HARDWARE_MODULE_ID, (const hw_module_t **)&mModule);
    if (err < 0) {
        MY_LOGE("hw_get_module failed, err %d\n", err);
        return false;
    }

    MY_LOGD("mModule = %p\n", mModule);
    MY_LOGD("module_api_version: 0x%x\n", mModule->common.module_api_version);

    if (mModule->common.module_api_version > CAMERA_MODULE_API_VERSION_2_4) {
        ret = mModule->init();
    }

    MY_LOGD("module name: %s\n", mModule->common.name);

    int numCameras = mModule->get_number_of_cameras();
    MY_LOGD("numCameras: %d\n", numCameras);

    char camera_id[10];
    snprintf(camera_id, sizeof(camera_id), "%d", id);

    ret = mModule->common.methods->open(&mModule->common, camera_id, (hw_device_t **)&mDevice);

    if (ret != OK) {
        MY_LOGE("Could not open camera 0: %d\n", ret);
        return false;
    }

    if (mDevice->ops->set_callbacks) {
        mDevice->ops->set_callbacks(mDevice,
                               __notify_cb,
                               __data_cb,
                               __data_cb_timestamp,
                               __get_memory,
                               this);
    }

    return  true;
}


void ImageProcessThread::disconnectCamera()
{
    if (mDevice->ops->disable_msg_type)
        mDevice->ops->disable_msg_type(mDevice, CAMERA_MSG_ALL_MSGS);

    if (mDevice->ops->stop_preview)
        mDevice->ops->stop_preview(mDevice);

    if (mDevice->ops->cancel_picture)
        mDevice->ops->cancel_picture(mDevice);

    if (mDevice->ops->release) {
        mDevice->ops->release(mDevice);
    }

    if (mDevice->common.close) {
        mDevice->common.close((hw_device_t*)mDevice);
    }

    mDevice = NULL;
    mModule = NULL;
}


bool ImageProcessThread::setupParameters()
{
    CameraParameters params;
    if (mDevice->ops->get_parameters) {
        char *temp = mDevice->ops->get_parameters(mDevice);
        String8 str_parms(temp);
        if (mDevice->ops->put_parameters)
            mDevice->ops->put_parameters(mDevice, temp);
        else
            free(temp);
        params.unflatten(str_parms);
    }

    params.set(MtkCameraParameters::KEY_CAMERA_MODE, mArgument->mi4CamMode);
    params.setPreviewSize(mArgument->mPreviewSize.width, mArgument->mPreviewSize.height);
    params.set(CameraParameters::KEY_PREVIEW_FORMAT, mArgument->ms8PrvFmt.string());


    if (OK != mDevice->ops->set_parameters(mDevice, params.flatten().string()))
    {
        CAM_LOGE("setParameters failed !!!\n");
        return  false;
    }

    return  true;
}


void ImageProcessThread::__notify_cb(int32_t msg_type, int32_t ext1,
                        int32_t ext2, void *user)
{
    MY_LOGD("msg_type %d, ext1 %d, ext2 %d", msg_type, ext1, ext2);
}


void ImageProcessThread::__data_cb(int32_t msg_type,
                      const camera_memory_t *data, unsigned int index,
                      camera_frame_metadata_t *metadata,
                      void *user)
{
    static int i = 0;

    MY_LOGD_IF(i%100==0, "i:%d msg_type %d, index %d", i, msg_type, index);
    sp<CameraHeapMemory> mem(static_cast<CameraHeapMemory *>(data->handle));
    if (index >= mem->mNumBufs) {
        MY_LOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
             index, mem->mNumBufs);
        return;
    }

    const sp<IMemory>& dataPtr = mem->mBuffers[index];
    ssize_t offset;
    size_t size;
    sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
    uint8_t* pBase = (uint8_t *)heap->base() + offset;

    if ( 0 == (msg_type & CAMERA_MSG_PREVIEW_FRAME) ){
        return;
    }

    //writeTouchPoints(mVirtualTouch, pBase, mPointsFd);

#if 0
    if (i%100==0) {
        String8 filename = String8::format("sdcard/preview_%05d.yuv", i);
        saveBufToFile(filename, pBase, size);
    }
#endif
    i++;
}


void ImageProcessThread::__data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                         const camera_memory_t *data, unsigned index,
                         void *user)
{
    MY_LOGD("msg_type %d, index %d", msg_type, index);
}


camera_memory_t* ImageProcessThread::__get_memory(int fd, size_t buf_size, uint_t num_bufs,
                                     void *user __attribute__((unused)))
{
    CameraHeapMemory *mem;
    MY_LOGD("fd %d, buf_size %d, num_bufs %d", fd, buf_size, num_bufs);
    if (fd < 0)
        mem = new CameraHeapMemory(buf_size, num_bufs);
    else
        mem = new CameraHeapMemory(fd, buf_size, num_bufs);
    mem->incStrong(mem);
    return &mem->handle;
}


void ImageProcessThread::__put_memory(camera_memory_t *data)
{
    MY_LOGD("data %p", data);
    if (!data)
        return;

    CameraHeapMemory *mem = static_cast<CameraHeapMemory *>(data->handle);
    mem->decStrong(mem);
}

