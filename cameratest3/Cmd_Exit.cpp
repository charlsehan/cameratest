#include <utils/Log.h>
#include <cutils/memory.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/Command.h"
#if defined(COMMAND_exit)

using namespace android;

namespace NSCmd_exit {
struct CmdImp : public CmdBase
{
    static bool                 isInstantiate;

                                CmdImp(char const* szCmdName)
                                    : CmdBase(szCmdName)
                                {}

    virtual bool                execute(Vector<String8>& rvCmd);
};

bool CmdImp::isInstantiate = CmdMap::inst().addCommand(COMMAND_exit, new CmdImp(COMMAND_exit));
};  // NSCmd_exit
using namespace NSCmd_exit;


bool
CmdImp::
execute(Vector<String8>& rvCmd)
{
    MY_LOGW("exit......");
    ::exit(0);
    return  true;
}

#endif  //  HAVE_COMMAND_xxx

