#include <utils/Log.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <cutils/memory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/IGraphicBufferProducer.h>

#include <Camera.h>
#include <CameraParameters.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>

#include <system/window.h>
#include <system/camera.h>
#include <hardware/camera.h>

#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/Command.h"

#if defined(COMMAND_test)

using namespace android;

#include <sys/prctl.h>

namespace NSCmd_test {
char const
gPromptText[] = {
    "\n"
    "\n test <action> <optional arguments...>"
    "\n "
    "\n where <action> may be one of the following:"
    "\n  <-h>                       show this help"
    "\n  <start>                    start preview and test start."
    "\n  <stop>                     stop preview and test exit."
    "\n "
    "\n where <optional arguments...> may be a combination of the followings:"
    "\n  <-open-id=0>               open id; 0 by default (main camera)."
    "\n  <-preview-size=640x480>    preview size; 640x480 by default."
    "\n  <-display=on>              display 'on' / 'off'; 'on' by default."
    "\n  <-display-orientation=90>  display orientation; 0 by default."
    "\n"
};

#if 0
char const gPromptText_bak[] = {
    "\n"
    "\n test_prv_cb <action> <optional arguments...>"
    "\n "
    "\n where <action> may be one of the following:"
    "\n  <-h>                       help"
    "\n  <startPreview>             start preview and test start."
    "\n  <stopPreview>              stop preview and test exit."
    "\n "
    "\n where <optional arguments...> may be a combination of the followings:"
    "\n  <-app-mode=Default>        app mode; 'Default' by default."
    "\n                             -> 'Default' 'MtkEng' 'MtkAtv' 'MtkStereo' 'MtkVt'"
    "\n  <-open-id=0>               open id; 0 by default (main camera)."
    "\n  <-cam-mode=1>              camera mode in KEY_CAMERA_MODE; 0 by default."
    "\n                             '0' refers to CAMERA_MODE_NORMAL"
    "\n                             '1' refers to CAMERA_MODE_MTK_PRV"
    "\n                             '2' refers to CAMERA_MODE_MTK_VDO"
    "\n                             '3' refers to CAMERA_MODE_MTK_VT"
    "\n  <-preview-size=640x480>    preview size; 640x480 by default."
    "\n  <-display=on>              display 'on' / 'off'; 'on' by default."
    "\n  <-display-orientation=90>  display orientation; 90 by default."
    "\n  <-formt=YV12>              preview format: YV12, NV21; by default YV12"
};
#endif


struct CmdImp : public CmdBase, public Thread, public CameraListener
{
    static bool                 isInstantiate;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CmdBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Good place to do one-time initializations
    virtual status_t            readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CameraListener Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual void                notify(int32_t msgType, int32_t ext1, int32_t ext2) {}
    virtual void                postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata);
    virtual void                postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {}
    virtual void                postRecordingFrameHandleTimestamp(nsecs_t timestamp, native_handle_t* handle) {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                struct Argument : LightRefBase<Argument>
                                {
                                    String8                     ms8AppMode;
                                    int32_t                     mi4CamMode;
                                    int32_t                     mOpenId;
                                    Size                        mPreviewSize;
                                    bool                        mDisplayOn;
                                    int32_t                     mDisplayOrientation;
                                    String8                     ms8PrvFmt;
                                };

protected:
    virtual bool                onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);
    virtual bool                onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument);

    virtual bool                onStartPreview(sp<Argument> pArgument);
    virtual bool                onStopPreview();
    virtual bool                onReset();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Surface)
    virtual bool                initSurface();
    virtual void                uninitSurface();

protected:  ////                Data Members (Surface)
    int32_t                     mi4SurfaceID;
    sp<SurfaceComposerClient>   mpSurfaceClient;
    sp<SurfaceControl>          mpSurfaceControl;
    sp<Surface>                 mpSurface;
    sp<IGraphicBufferProducer>  mpGbp;
    ANativeWindow*              mpWindow;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Camera)
    virtual bool                connectCamera(int id = 0);
    virtual void                disconnectCamera();
    virtual bool                setupParameters(sp<Camera> spCamera);

protected:  ////                Data Members (Camera)
    sp<Camera>                  mpCamera;

protected:  ////                Data Members (Parameters)
    sp<Argument>                mpArgument;

    Mutex                       mThreadMutex;
    Condition                   mThreadCond;
    int32_t volatile            mThreadRunning;

#if 1
private:
    camera_module_t *mModule;
    camera_device_t *mDevice;

    static void __notify_cb(int32_t msg_type, int32_t ext1,
                            int32_t ext2, void *user)
    {
        MY_LOGD("msg_type %d, ext1 %d, ext2 %d", msg_type, ext1, ext2);
    }

    static void __data_cb(int32_t msg_type,
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
    
    
        MY_LOGD_IF(i%100==0, "i:%d msg_type=0x%x CAMERA_MSG_PREVIEW_FRAME?%d base/size=%p/%d", i, msg_type, (msg_type & CAMERA_MSG_PREVIEW_FRAME), pBase, size);
        if ( 0 == (msg_type & CAMERA_MSG_PREVIEW_FRAME) )
        {
            return;
        }
#if 0
        if (i%100==0) {
            String8 filename = String8::format("sdcard/preview_%02d.yuv", i);
            saveBufToFile(filename, pBase, size);
        }
#endif
        i++;
    }

    static void __data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                             const camera_memory_t *data, unsigned index,
                             void *user)
    {
        MY_LOGD("msg_type %d, index %d", msg_type, index);
    }

    // This is a utility class that combines a MemoryHeapBase and a MemoryBase
    // in one.  Since we tend to use them in a one-to-one relationship, this is
    // handy.

    class CameraHeapMemory : public RefBase {
    public:
        CameraHeapMemory(int fd, size_t buf_size, uint_t num_buffers = 1) :
                         mBufSize(buf_size),
                         mNumBufs(num_buffers)
        {
            mHeap = new MemoryHeapBase(fd, buf_size * num_buffers);
            commonInitialization();
        }

        CameraHeapMemory(size_t buf_size, uint_t num_buffers = 1) :
                         mBufSize(buf_size),
                         mNumBufs(num_buffers)
        {
            mHeap = new MemoryHeapBase(buf_size * num_buffers);
            commonInitialization();
        }

        void commonInitialization()
        {
            handle.data = mHeap->base();
            handle.size = mBufSize * mNumBufs;
            handle.handle = this;

            mBuffers = new sp<MemoryBase>[mNumBufs];
            for (uint_t i = 0; i < mNumBufs; i++)
                mBuffers[i] = new MemoryBase(mHeap,
                                             i * mBufSize,
                                             mBufSize);

            handle.release = __put_memory;
        }

        virtual ~CameraHeapMemory()
        {
            delete [] mBuffers;
        }

        size_t mBufSize;
        uint_t mNumBufs;
        sp<MemoryHeapBase> mHeap;
        sp<MemoryBase> *mBuffers;

        camera_memory_t handle;
    };

    static camera_memory_t* __get_memory(int fd, size_t buf_size, uint_t num_bufs,
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

    static void __put_memory(camera_memory_t *data)
    {
        MY_LOGD("data %p", data);
        if (!data)
            return;

        CameraHeapMemory *mem = static_cast<CameraHeapMemory *>(data->handle);
        mem->decStrong(mem);
    }

#endif
};

bool CmdImp::isInstantiate = CmdMap::inst().addCommand(COMMAND_test, new CmdImp(COMMAND_test));
};  // NSCmd_test
using namespace NSCmd_test;


bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    sp<Argument> pArgument = new Argument;
    onParseArgumentCommand(rvCmd, pArgument);
    return  onParseActionCommand(rvCmd, pArgument);
}


bool
CmdImp::
onParseArgumentCommand(Vector<String8>& rvCmd, sp<Argument> pArgument)
{
    //  (1) Set default.
    pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
    pArgument->mi4CamMode = MtkCameraParameters::CAMERA_MODE_MTK_PRV;
    pArgument->mOpenId = 0;
    pArgument->mPreviewSize = Size(640, 480);
    pArgument->mDisplayOrientation = 0;
    pArgument->mDisplayOn = true;
    pArgument->ms8PrvFmt = String8(CameraParameters::PIXEL_FORMAT_YUV420P);

    //  (2) Start to parse commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        String8 key, val;
        if  ( ! parseOneCmdArgument(s8Cmd, key, val) ) {
            continue;
        }
        //MY_LOGD("<key/val>=<%s/%s>", key.string(), val.string());

        if  ( key == "-app-mode" ) {
            if  ( val == "MtkEng" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ENG);
                continue;
            }
            if  ( val == "MtkAtv" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ATV);
                continue;
            }
            if  ( val == "MtkStereo" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_STEREO);
                continue;
            }
            if  ( val == "MtkVt" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_VT);
                continue;
            }
#if 0
            if  ( val == "MtkPhoto" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_PHOTO);
                continue;
            }
            if  ( val == "MtkVideo" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO);
                continue;
            }
            if  ( val == "MtkZsd" ) {
                pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_MTK_ZSD);
                continue;
            }
#endif
            pArgument->ms8AppMode = String8(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
            continue;
        }

        if  ( key == "-cam-mode" ) {
            pArgument->mi4CamMode = ::atoi(val);
            continue;
        }

        if  ( key == "-open-id" ) {
            pArgument->mOpenId = ::atoi(val);
            continue;
        }

        if  ( key == "-preview-size" ) {
            ::sscanf(val.string(), "%dx%d", &pArgument->mPreviewSize.width, &pArgument->mPreviewSize.height);
            MY_LOGD("preview-size : %d %d", pArgument->mPreviewSize.width, pArgument->mPreviewSize.height);
            continue;
        }

        if  ( key == "-display-orientation" ) {
            pArgument->mDisplayOrientation = ::atoi(val);
            continue;
        }

        if  ( key == "-display" ) {
            pArgument->mDisplayOn = (val == "on") ? 1 : 0;
            continue;
        }

        if  ( key == "-format") {
            if  ( val == "NV21" ) {
                pArgument->ms8PrvFmt = String8(CameraParameters::PIXEL_FORMAT_YUV420SP);
            }
            if  ( val == "YV12" ) {
                pArgument->ms8PrvFmt = String8(CameraParameters::PIXEL_FORMAT_YUV420P);
            }
            MY_LOGD("format : %s", pArgument->ms8PrvFmt.string());
        }
    }
    return  true;
}


bool
CmdImp::
onParseActionCommand(Vector<String8>& rvCmd, sp<Argument> pArgument)
{
    if (rvCmd.size() == 1) {
        // no action, show help
        printf("%s", gPromptText);
        return  true;
    }
    //  (1) Start to parse ACTION commands.
    for (size_t i = 1; i < rvCmd.size(); i++)
    {
        String8 const& s8Cmd = rvCmd[i];
        //
        if  ( s8Cmd == "-h" ) {
            printf("%s", gPromptText);
            return  true;
        }
        //
        if  ( s8Cmd == "start" ) {
            return  onStartPreview(pArgument);
        }
        //
        if  ( s8Cmd == "stop" ) {
            return  onStopPreview();
        }
    }
    return  false;
}


bool
CmdImp::
onStartPreview(sp<Argument> pArgument)
{
    if  ( 1 == ::android_atomic_release_load(&mThreadRunning) ) {
        MY_LOGW("the test has been starting before...skip this command.");
        return  false;
    }

    onReset();
    ::android_atomic_release_store(1, &mThreadRunning);
    mpArgument = pArgument;
    return  (OK == run("CmdImp::onStartPreview"));
}


bool
CmdImp::
onStopPreview()
{
    requestExit();
    {
        Mutex::Autolock _lock(mThreadMutex);
        ::android_atomic_release_store(0, &mThreadRunning);
        mThreadCond.broadcast();
    }

    return  (OK == join());
}


bool
CmdImp::
onReset()
{
    ::android_atomic_release_store(0, &mThreadRunning);
    return  true;
}


// Good place to do one-time initializations
status_t
CmdImp::
readyToRun()
{
    ::prctl(PR_SET_NAME,"CmdImp", 0, 0, 0);

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


bool
CmdImp::
threadLoop()
{
    status_t status;
    bool ret = false;

    ret =
            connectCamera(mpArgument->mOpenId)
        &&  setupParameters(mpCamera)
        &&  initSurface()
            ;
    if  ( ! ret )
    {
        goto lbExit;
    }

#if 0
    if  ( mpArgument->mDisplayOn ) {
        MY_LOGD("setPreviewDisplay, orientation=%d", mpArgument->mDisplayOrientation);
        mpCamera->setPreviewTarget(mpGbp);
        mpCamera->sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION, mpArgument->mDisplayOrientation, 0);
    }
#else
    if  ( mpArgument->mDisplayOn ) {
        //TODO: use mDevice interface
    }
#endif
#if 0
    mpCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_ENABLE_MASK);

    mpCamera->startPreview();
    {
        Mutex::Autolock _lock(mThreadMutex);
        while   ( 1 == ::android_atomic_release_load(&mThreadRunning) )
        {
            nsecs_t nsTimeoutToWait = 30LL*1000LL*1000LL*1000LL;//wait 30 sec.
            MY_LOGD("Start to wait %lld sec...", nsTimeoutToWait/1000000000LL);
            status_t status = mThreadCond.waitRelative(mThreadMutex, nsTimeoutToWait);
        }
    }
    mpCamera->stopPreview();

#else
    if (mDevice->ops->enable_msg_type)
        mDevice->ops->enable_msg_type(mDevice, CAMERA_MSG_PREVIEW_FRAME);

    MY_LOGD("after enable_msg_type: %d\n", CAMERA_MSG_PREVIEW_FRAME);

    if (mDevice->ops->start_preview)
        mDevice->ops->start_preview(mDevice);

    {
        Mutex::Autolock _lock(mThreadMutex);
        while   ( 1 == ::android_atomic_release_load(&mThreadRunning) )
        {
            nsecs_t nsTimeoutToWait = 30LL*1000LL*1000LL*1000LL;//wait 30 sec.
            MY_LOGD("Start to wait %lld sec...", nsTimeoutToWait/1000000000LL);
            status_t status = mThreadCond.waitRelative(mThreadMutex, nsTimeoutToWait);
        }
    }

    if (mDevice->ops->stop_preview)
        mDevice->ops->stop_preview(mDevice);
#endif



lbExit:
    uninitSurface();
    disconnectCamera();
    onReset();
    return  false;
}


bool
CmdImp::
initSurface()
{
#if 0
    mi4SurfaceID = 0;

    // create a client to surfaceflinger
    mpSurfaceClient = new SurfaceComposerClient();

    mpSurfaceControl = mpSurfaceClient->createSurface(
        String8("surface"), 640, 480, PIXEL_FORMAT_RGBA_8888, 0
    );
    SurfaceComposerClient::openGlobalTransaction();
    mpSurfaceControl->setLayer(100000);
    mpSurfaceControl->show();
    SurfaceComposerClient::closeGlobalTransaction();
    // pretend it went cross-process
    Parcel parcel;
    SurfaceControl::writeSurfaceToParcel(mpSurfaceControl, &parcel);
    parcel.setDataPosition(0);

    mpSurface = mpSurfaceControl->getSurface();
    mpWindow = mpSurface.get();

    if(mpSurface != NULL) {
        mpGbp = mpSurface->getIGraphicBufferProducer();
    }

    CAM_LOGD("setupSurface: %p", mpSurface.get());
    return  (mpSurface != 0);
#else
    return true;
#endif
}


void
CmdImp::
uninitSurface()
{
    mpWindow = NULL;
    mpSurface = 0;
    mpSurfaceControl = 0;
    mpSurfaceClient = 0;
}


bool
CmdImp::
connectCamera(int id)
{
#if 0
    mpCamera = Camera::connect(id, String16("CamTest"), Camera::USE_CALLING_UID, Camera::USE_CALLING_PID);
    if  ( mpCamera == 0 )
    {
        MY_LOGE("Camera::connect, id(%d)", id);
        return  false;
    }

    mpCamera->setListener(this);

    MY_LOGD("Camera::connect, id(%d), camera(%p)", id, mpCamera.get());
#else
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

#endif

    return  true;
}


void
CmdImp::
disconnectCamera()
{
#if 0
    if  ( mpCamera != 0 )
    {
        MY_LOGD("Camera::disconnect, camera(%p)", mpCamera.get());
        mpCamera->disconnect();
        mpCamera = NULL;
    }
#else
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
#endif
}


bool
CmdImp::
setupParameters(sp<Camera> spCamera)
{
#if 0
    CameraParameters params(spCamera->getParameters());

    params.set(MtkCameraParameters::KEY_CAMERA_MODE, mpArgument->mi4CamMode);

    params.setPreviewSize(mpArgument->mPreviewSize.width, mpArgument->mPreviewSize.height);

    params.set(CameraParameters::KEY_PREVIEW_FORMAT, mpArgument->ms8PrvFmt.string());

    if  (OK != spCamera->setParameters(params.flatten()))
    {
        CAM_LOGE("setParameters failed !!!\n");
        return  false;
    }
#else
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

    params.set(MtkCameraParameters::KEY_CAMERA_MODE, mpArgument->mi4CamMode);
    params.setPreviewSize(mpArgument->mPreviewSize.width, mpArgument->mPreviewSize.height);
    params.set(CameraParameters::KEY_PREVIEW_FORMAT, mpArgument->ms8PrvFmt.string());


    if (OK != mDevice->ops->set_parameters(mDevice, params.flatten().string()))
    {
        CAM_LOGE("setParameters failed !!!\n");
        return  false;
    }
#endif
    return  true;
}


void
CmdImp::
postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
    ssize_t offset;
    size_t size;
    sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
    uint8_t* pBase = (uint8_t *)heap->base() + offset;

    static int i = 0;

    MY_LOGD_IF(i%100==0, "i:%d msgType=%x CAMERA_MSG_PREVIEW_FRAME?%d base/size=%p/%d", i, msgType, (msgType & CAMERA_MSG_PREVIEW_FRAME), pBase, size);
    if  ( 0 == (msgType & CAMERA_MSG_PREVIEW_FRAME) )
    {
        return;
    }
#if 0
    if (i%100==0) {
        String8 filename = String8::format("sdcard/prv%dx%d_%02d.yuv", mpArgument->mPreviewSize.width, mpArgument->mPreviewSize.height, i);
        saveBufToFile(filename, pBase, size);
    }
#endif
    i++;
}

#endif  //  HAVE_COMMAND_xxx

