#include <utils/Log.h>
#include <cutils/memory.h>
#include "cutils/properties.h"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include "inc/CamLog.h"
#include "inc/Utils.h"

using namespace android;

static
Vector<String8>
queryCommandVector(char const cCmd[], int fd)
{
    Vector<String8> vCommand;

    char const *s1 = cCmd, *s2 = 0;
    while ( 0 != s1 && 0 != (*s1) ) {
        if  ( ' ' == (*s1) ) {
            continue;
        }

        s2 = ::strchr(s1, ' ');
        if  ( s2 == 0 ) {
            // If there's no space, this is the last item.
            vCommand.push_back(String8(s1));
            break;
        }
        vCommand.push_back(String8(s1, (size_t)(s2-s1)));
        s1 = s2 + 1;
    }

    return  vCommand;
}

#define INPUT_FROM_COMMAND_LINE
int main(int argc, char** argv)
{
    char cCmds[256] = {0};

    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    //CmdTest cmdtest = new CmdTest(COMMAND_test);

    while (1)
    {
        printf("[camtest]$ ");
        ::fgets(cCmds, sizeof(cCmds), stdin);
        cCmds[::strlen(cCmds)-1] = '\0'; //remove the '\n'
        if  ( 0 == ::strcmp(cCmds, "start") ) {
            sprintf(cCmds, "test start");
        } else if  ( 0 == ::strcmp(cCmds, "stop") ) {
            sprintf(cCmds, "test stop");
        }

        Vector<String8> vCmd = queryCommandVector(cCmds, 1);
        if  ( vCmd.empty() ) {
            continue;
        }

    }
    MY_LOGD("exit while""\n");

    IPCThreadState::self()->joinThreadPool();
    return 0;
}

