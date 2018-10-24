#ifndef _CAMERA_TEST_COMMAND_H_
#define _CAMERA_TEST_COMMAND_H_

#define COMMAND_exit    "exit"    //  exit
#define COMMAND_test    "test"    //  test preview with callback


namespace android {

class CmdBase : public virtual RefBase
{
protected:  ////
    String8 const   ms8CmdName;

public:     ////    Instantiation.
    virtual         ~CmdBase() {}
                    CmdBase(char const* szCmdName)
                        : ms8CmdName(String8(szCmdName))
                    {
                    }

public:     ////    Utils.
    String8 const&  getName() const { return ms8CmdName; }

    bool            parseOneCmdArgument(String8 const& rCmdArg, String8& rKey, String8& rVal)
                    {
                        const char *a = rCmdArg.string();
                        const char *b;

                        // Find the bounds of the key name.
                        b = ::strchr(a, '=');
                        if (b == 0)
                            return false;

                        // Create the key string.
                        rKey = String8(a, (size_t)(b-a));

                        // Find the value.
                        a = b+1;
                        rVal = String8(a);
                        return  true;
                    }

public:     ////    Interfaces.
    virtual bool    execute(Vector<String8>& rvCmd) = 0;
};


class CmdMap
{
protected:  ////    Data Members.
    typedef DefaultKeyedVector< String8, sp<CmdBase> >  CmdMap_t;
    CmdMap_t        mvCmdMap;

public:     ////    Interfaces.
    static CmdMap&  inst();
    bool            execute(Vector<String8>& rvCmd);
    void            help();
    bool            addCommand(char const*const pszName, CmdBase*const pCmdInstance);

public:     ////    Instantiation.
                    CmdMap();
};


};  // namespace android
#endif  //_CAMERA_TEST_COMMAND_H_

