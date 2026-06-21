#include <windows.h>

typedef void   (*fn_scriptWait)(DWORD time);
typedef void   (*fn_scriptRegister)(HMODULE module, void(*)());
typedef void   (*fn_scriptUnregister)(HMODULE module);
typedef void   (*fn_nativeInit)(UINT64 hash);
typedef void   (*fn_nativePush64)(UINT64 val);
typedef UINT64*(*fn_nativeCall)();

static fn_scriptWait       g_scriptWait       = nullptr;
static fn_scriptRegister   g_scriptRegister   = nullptr;
static fn_scriptUnregister g_scriptUnregister = nullptr;
static fn_nativeInit       g_nativeInit       = nullptr;
static fn_nativePush64     g_nativePush64     = nullptr;
static fn_nativeCall       g_nativeCall       = nullptr;
static HMODULE             g_self             = nullptr;
static HMODULE             g_shv              = nullptr;

static void WAIT(DWORD ms) { g_scriptWait(ms); }

static void callNative(UINT64 hash)
{
    g_nativeInit(hash);
    g_nativeCall();
}
static void callNative1(UINT64 hash, UINT64 a0)
{
    g_nativeInit(hash);
    g_nativePush64(a0);
    g_nativeCall();
}
static void callNative2(UINT64 hash, UINT64 a0, UINT64 a1)
{
    g_nativeInit(hash);
    g_nativePush64(a0);
    g_nativePush64(a1);
    g_nativeCall();
}
static UINT64 callNativeRet2(UINT64 hash, UINT64 a0, UINT64 a1)
{
    g_nativeInit(hash);
    g_nativePush64(a0);
    g_nativePush64(a1);
    UINT64* res = g_nativeCall();
    return res ? *res : 0;
}

static void REMOVE_IPL(const char* name)
{
    callNative1(0xEE6C5AD3ECE0A82DULL, (UINT64)(uintptr_t)name);
}
static void REQUEST_IPL(const char* name)
{
    callNative1(0x41B4893843BBDB74ULL, (UINT64)(uintptr_t)name);
}
static void SET_NOTIFICATION_TEXT_ENTRY(const char* type)
{
    callNative1(0x202709F4C58A0424ULL, (UINT64)(uintptr_t)type);
}
static void ADD_TEXT_COMPONENT_STRING(const char* text)
{
    callNative1(0x6C188BE134E074AAULL, (UINT64)(uintptr_t)text);
}
static void DRAW_NOTIFICATION(BOOL blink, BOOL p1)
{
    callNative2(0x2ED7843F8F801023ULL, (UINT64)blink, (UINT64)p1);
}
static void PLAY_SOUND_FRONTEND(int soundId, const char* soundName, const char* setName, BOOL p3)
{
    g_nativeInit(0x67C540AA08E4A6F5ULL);
    g_nativePush64((UINT64)(INT64)soundId);
    g_nativePush64((UINT64)(uintptr_t)soundName);
    g_nativePush64((UINT64)(uintptr_t)setName);
    g_nativePush64((UINT64)p3);
    g_nativeCall();
}

static void ShowNotification(const char* msg)
{
    PLAY_SOUND_FRONTEND(-1, "PROPERTY_PURCHASE", "HUD_AWARDS", FALSE);
    SET_NOTIFICATION_TEXT_ENTRY("STRING");
    ADD_TEXT_COMPONENT_STRING(msg);
    DRAW_NOTIFICATION(FALSE, TRUE);
}

static void ScriptMain()
{
    WAIT(3000);

    REMOVE_IPL("v_trailertrash");
    WAIT(500);
    REMOVE_IPL("v_trailer");
    WAIT(500);
    REMOVE_IPL("trevorstrailertrash");
    WAIT(500);
    REMOVE_IPL("trevorstrailer");
    WAIT(500);
    REQUEST_IPL("v_trailertidy");
    WAIT(500);
    REQUEST_IPL("trevorstrailertidy");
    WAIT(1000);

    ShowNotification("~g~TrevorTidy~w~ loaded successfully!");

    WAIT(MAXDWORD);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        g_self = hModule;
        g_shv  = GetModuleHandleA("ScriptHookV.dll");
        if (!g_shv) return FALSE;

        g_scriptWait       = (fn_scriptWait)       GetProcAddress(g_shv, "?scriptWait@@YAXK@Z");
        g_scriptRegister   = (fn_scriptRegister)   GetProcAddress(g_shv, "?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z");
        g_scriptUnregister = (fn_scriptUnregister) GetProcAddress(g_shv, "?scriptUnregister@@YAXPEAUHINSTANCE__@@Z");
        g_nativeInit       = (fn_nativeInit)       GetProcAddress(g_shv, "?nativeInit@@YAX_K@Z");
        g_nativePush64     = (fn_nativePush64)     GetProcAddress(g_shv, "?nativePush64@@YAX_K@Z");
        g_nativeCall       = (fn_nativeCall)       GetProcAddress(g_shv, "?nativeCall@@YAPEA_KXZ");

        if (!g_scriptWait || !g_scriptRegister || !g_nativeInit || !g_nativePush64 || !g_nativeCall)
            return FALSE;

        g_scriptRegister(hModule, ScriptMain);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        if (g_scriptUnregister && g_shv)
            g_scriptUnregister(g_self);
    }
    return TRUE;
}
