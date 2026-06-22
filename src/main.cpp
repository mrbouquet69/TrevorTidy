#include <windows.h>
#include <cstdint>

typedef void (*fn_scriptWait)(DWORD time);
typedef void (*fn_scriptRegister)(HMODULE module, void(*)());
typedef void (*fn_scriptUnregister)(HMODULE module);
typedef void (*fn_nativeInit)(uint64_t hash);
typedef void (*fn_nativePush64)(uint64_t val);
typedef uint64_t*(*fn_nativeCall)();

static fn_scriptWait g_scriptWait = nullptr;
static fn_scriptRegister g_scriptRegister = nullptr;
static fn_scriptUnregister g_scriptUnregister = nullptr;
static fn_nativeInit g_nativeInit = nullptr;
static fn_nativePush64 g_nativePush64 = nullptr;
static fn_nativeCall g_nativeCall = nullptr;

static HMODULE g_self = nullptr;
static HMODULE g_shv = nullptr;

static inline void WAIT(DWORD ms) { 
    if (g_scriptWait) g_scriptWait(ms); 
}

static inline void callNative1(uint64_t hash, uint64_t val) {
    if (g_nativeInit && g_nativePush64 && g_nativeCall) {
        g_nativeInit(hash);
        g_nativePush64(val);
        g_nativeCall();
    }
}

static inline void callNative2(uint64_t hash, uint64_t val1, uint64_t val2) {
    if (g_nativeInit && g_nativePush64 && g_nativeCall) {
        g_nativeInit(hash);
        g_nativePush64(val1);
        g_nativePush64(val2);
        g_nativeCall();
    }
}

static void REMOVE_IPL(const char* name) { 
    callNative1(0xEE6C5AD3ECE0A82DULL, reinterpret_cast<uintptr_t>(name)); 
}

static void REQUEST_IPL(const char* name) { 
    callNative1(0x41B4893843BBDB74ULL, reinterpret_cast<uintptr_t>(name)); 
}

static void SHOW_NOTIFICATION(const char* msg) {
    if (g_nativeInit && g_nativePush64 && g_nativeCall) {
        g_nativeInit(0x67C540AA08E4A6F5ULL);
        g_nativePush64(static_cast<uint64_t>(-1));
        g_nativePush64(reinterpret_cast<uintptr_t>("PROPERTY_PURCHASE"));
        g_nativePush64(reinterpret_cast<uintptr_t>("HUD_AWARDS"));
        g_nativePush64(FALSE);
        g_nativeCall();
    }
    
    callNative1(0x202709F4C58A0424ULL, reinterpret_cast<uintptr_t>("STRING"));
    callNative1(0x6C188BE134E074AAULL, reinterpret_cast<uintptr_t>(msg));
    callNative2(0x2ED7843F8F801023ULL, FALSE, TRUE);
}

static void ScriptMain() {
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
    REQUEST_IPL("trevorstrailertidy");  // need to load the ymap along with the MLO other wise it wont work
    WAIT(1000);
    
    SHOW_NOTIFICATION("~g~TrevorTidy~w~ loaded successfully!");
    while (true) {
        WAIT(86400000); // 24-hour cycle loop is safer than MAXDWORD overhead
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = hModule;
        g_shv = GetModuleHandleA("ScriptHookV.dll");
        if (!g_shv) return FALSE;

        g_scriptWait = (fn_scriptWait)GetProcAddress(g_shv, "?scriptWait@@YAXK@Z");
        g_scriptRegister = (fn_scriptRegister)GetProcAddress(g_shv, "?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z");
        g_scriptUnregister = (fn_scriptUnregister)GetProcAddress(g_shv, "?scriptUnregister@@YAXPEAUHINSTANCE__@@Z");
        g_nativeInit = (fn_nativeInit)GetProcAddress(g_shv, "?nativeInit@@YAX_K@Z");
        g_nativePush64 = (fn_nativePush64)GetProcAddress(g_shv, "?nativePush64@@YAX_K@Z");
        g_nativeCall = (fn_nativeCall)GetProcAddress(g_shv, "?nativeCall@@YAPEA_KXZ");

        if (!g_scriptWait || !g_scriptRegister || !g_scriptUnregister || !g_nativeInit || !g_nativePush64 || !g_nativeCall) {
            return FALSE;
        }

        g_scriptRegister(hModule, ScriptMain);
    } 
    else if (reason == DLL_PROCESS_DETACH) {
        if (g_scriptUnregister && g_shv) {
            g_scriptUnregister(g_self);
        }
    }
    return TRUE;
}
