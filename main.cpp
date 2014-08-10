#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "include/SDK/XPLMDefs.h"
#include "include/session.h"
#include "include/version.h"

const char* plugin_version = "1.0.1-2";

#if IBM
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif
    PLUGIN_API int XPluginStart(char* name, char* signature, char* description);
    PLUGIN_API int XPluginEnable(void);
    PLUGIN_API void XPluginDisable(void);
    PLUGIN_API void XPluginStop(void);
    PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void* inParam);
#ifdef __cplusplus
}
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif

static session_t* session = 0;

PLUGIN_API int XPluginStart(char* name, char* signature, char* description)
{
    strcpy(name, "xcontrol (");
    strcat(name, plugin_version);
    strcat(name, ")");
    strcpy(signature, "copai.equipment.control");
    strcpy(description, "provides enhanced features covering Saitek  products, compiled on ");
    strcat(description, __DATE__);
    try
    {
        session = new session_t();
    }
    catch (const char* e)
    {
        if (!strcmp(e, "control.nonfatal"))
            return 0;
        else
            abort();
    }
    return 1;
}

PLUGIN_API int XPluginEnable(void)
{
    if (!session) return 0;
    return session->enable();
    return 0;
}

PLUGIN_API void XPluginDisable(void)
{
    if (!session) return;
    session->disable();
    return;
}

PLUGIN_API void XPluginStop(void)
{
    if (session) delete session;
    return;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void* inParam)
{
    return;
}
