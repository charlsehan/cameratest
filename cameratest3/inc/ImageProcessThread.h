#ifndef _IMAGE_PROCESS_THREAD_H_
#define _IMAGE_PROCESS_THREAD_H_

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

#include "vtouch/VirtualTouch.h"

using namespace android;

class ImageProcessThread : public Thread
{
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
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                ImageProcessThread();
                                ~ImageProcessThread();

    virtual bool                start();
    virtual bool                stop();

protected:
    virtual bool                reset();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
protected:  ////                Operations (Camera)
    virtual bool                connectCamera(int id = 0);
    virtual void                disconnectCamera();
    virtual bool                setupParameters();

protected:  ////                Data Members (Parameters)
                                struct Argument : LightRefBase<Argument>
                                {
                                    String8                     ms8AppMode;
                                    int32_t                     mi4CamMode;
                                    int32_t                     mOpenId;
                                    android::Size               mPreviewSize;
                                    bool                        mDisplayOn;
                                    int32_t                     mDisplayOrientation;
                                    String8                     ms8PrvFmt;
                                };

    sp<Argument>                mArgument;

    android::Mutex              mThreadMutex;
    Condition                   mThreadCond;
    int32_t volatile            mThreadRunning;

private:
    static int                  mPointsFd; //touch input device file point
    static VirtualTouch*        mVirtualTouch;

    camera_module_t *mModule;
    camera_device_t *mDevice;

    static void __notify_cb(int32_t msg_type, int32_t ext1,
                            int32_t ext2, void *user);

    static void __data_cb(int32_t msg_type,
                          const camera_memory_t *data, unsigned int index,
                          camera_frame_metadata_t *metadata,
                          void *user);

    static void __data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                             const camera_memory_t *data, unsigned index,
                             void *user);

    static camera_memory_t* __get_memory(int fd, size_t buf_size, uint_t num_bufs,
                                         void *user __attribute__((unused)));

    static void __put_memory(camera_memory_t *data);

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

};

#endif //_IMAGE_PROCESS_THREAD_H_
