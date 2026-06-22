#include <windows.h>
#include <cstdint>

typedef void      (*fn_scriptWait)(DWORD time);
typedef void      (*fn_scriptRegister)(HMODULE module, void(*)());
typedef void      (*fn_scriptUnregister)(HMODULE module);
typedef void      (*fn_nativeInit)(uint64_t hash);
typedef void      (*fn_nativePush64)(uint64_t val);
typedef uint64_t* (*fn_nativeCall)();

static fn_scriptWait       g_scriptWait       = nullptr;
static fn_scriptRegister   g_scriptRegister   = nullptr;
static fn_scriptUnregister g_scriptUnregister = nullptr;
static fn_nativeInit       g_nativeInit       = nullptr;
static fn_nativePush64     g_nativePush64     = nullptr;
static fn_nativeCall       g_nativeCall       = nullptr;

static HMODULE g_self = nullptr;
static HMODULE g_shv  = nullptr;

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

static inline uint64_t callNativeRet1(uint64_t hash, uint64_t val) {
    if (!g_nativeInit || !g_nativePush64 || !g_nativeCall) return 0;
    g_nativeInit(hash);
    g_nativePush64(val);
    uint64_t* r = g_nativeCall();
    return r ? *r : 0;
}

static inline uint64_t callNativeRetN(uint64_t hash, uint64_t* args, int argc) {
    if (!g_nativeInit || !g_nativePush64 || !g_nativeCall) return 0;
    g_nativeInit(hash);
    for (int i = 0; i < argc; i++) g_nativePush64(args[i]);
    uint64_t* r = g_nativeCall();
    return r ? *r : 0;
}

// ---- helpers to pass floats through the native interface ----

static inline uint64_t f2u(float f) {
    uint64_t out = 0;
    memcpy(&out, &f, sizeof(f));
    return out;
}

// ---- IPL / YMAP ----

static void REMOVE_IPL(const char* name) {
    callNative1(0xEE6C5AD3ECE0A82DULL, reinterpret_cast<uintptr_t>(name));
}

static void REQUEST_IPL(const char* name) {
    callNative1(0x41B4893843BBDB74ULL, reinterpret_cast<uintptr_t>(name));
}

// ---- notification ----

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

// ---- prop placement ----

static uint64_t GET_HASH_KEY(const char* name) {
    return callNativeRet1(0xD24D37CC275948CCULL, reinterpret_cast<uintptr_t>(name));
}

static void REQUEST_MODEL(uint64_t model) {
    callNative1(0x963D27A58DF860ACULL, model);
}

static bool HAS_MODEL_LOADED(uint64_t model) {
    return callNativeRet1(0x98A4EB5D89A0C952ULL, model) != 0;
}

static int CREATE_OBJECT(uint64_t model, float x, float y, float z) {
    uint64_t args[] = { model, f2u(x), f2u(y), f2u(z), FALSE, TRUE, FALSE };
    return static_cast<int>(callNativeRetN(0x509D5878EB39E842ULL, args, 7));
}

static void SET_ENTITY_QUATERNION(int entity, float x, float y, float z, float w) {
    uint64_t args[] = {
        static_cast<uint64_t>(entity),
        f2u(x), f2u(y), f2u(z), f2u(w)
    };
    callNativeRetN(0x77B21BE7AC540F07ULL, args, 5);
}

static void FREEZE_ENTITY_POSITION(int entity, BOOL toggle) {
    callNative2(0x428CA6DBD1094446ULL,
        static_cast<uint64_t>(entity),
        static_cast<uint64_t>(toggle));
}

static void SET_ENTITY_AS_MISSION_ENTITY(int entity, BOOL value, BOOL byThisScript) {
    uint64_t args[] = {
        static_cast<uint64_t>(entity),
        static_cast<uint64_t>(value),
        static_cast<uint64_t>(byThisScript)
    };
    callNativeRetN(0xAD738C3085FE7E11ULL, args, 3);
}

struct SignPlacement {
    float px, py, pz;
    float qx, qy, qz, qw;
};

static const SignPlacement SIGNS[] = {
    { 1968.187f, 3818.209f, 34.0987f,   0.0f,        0.0f,       0.258819f,  0.9659259f },
    { 1967.729f, 3817.982f, 34.27068f, -0.1830128f,  0.6830127f, 0.1830127f, 0.6830128f },
    { 1971.536f, 3815.579f, 34.11098f,  0.0f,        0.0f,       0.8660254f, 0.5f       },
    { 1972.388f, 3819.389f, 33.84687f,  0.0f,        0.0f,       0.8660254f, 0.5000001f },
    { 1974.774f, 3822.046f, 33.95044f, -0.1830128f,  0.6830127f, 0.1830127f, 0.6830128f },
};

static void PlaceSigns() {
    uint64_t model = GET_HASH_KEY("prop_cs_protest_sign_04a");
    REQUEST_MODEL(model);
    for (int i = 0; i < 100 && !HAS_MODEL_LOADED(model); i++)
        WAIT(100);

    for (int i = 0; i < 5; i++) {
        const SignPlacement& s = SIGNS[i];
        int obj = CREATE_OBJECT(model, s.px, s.py, s.pz);
        if (obj) {
            SET_ENTITY_AS_MISSION_ENTITY(obj, TRUE, TRUE);
            SET_ENTITY_QUATERNION(obj, s.qx, s.qy, s.qz, s.qw);
            FREEZE_ENTITY_POSITION(obj, TRUE);
        }
    }
}

// ---- entry ----

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
    REQUEST_IPL("trevorstrailertidy");
    WAIT(1000);

    PlaceSigns();

    SHOW_NOTIFICATION("~g~TrevorTidy~w~ loaded successfully!");

    while (true) {
        WAIT(86400000); // 24-hour cycle loop is safer than MAXDWORD overhead
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = hModule;
        g_shv  = GetModuleHandleA("ScriptHookV.dll");
        if (!g_shv) return FALSE;

        g_scriptWait       = (fn_scriptWait)       GetProcAddress(g_shv, "?scriptWait@@YAXK@Z");
        g_scriptRegister   = (fn_scriptRegister)   GetProcAddress(g_shv, "?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z");
        g_scriptUnregister = (fn_scriptUnregister) GetProcAddress(g_shv, "?scriptUnregister@@YAXPEAUHINSTANCE__@@Z");
        g_nativeInit       = (fn_nativeInit)       GetProcAddress(g_shv, "?nativeInit@@YAX_K@Z");
        g_nativePush64     = (fn_nativePush64)     GetProcAddress(g_shv, "?nativePush64@@YAX_K@Z");
        g_nativeCall       = (fn_nativeCall)       GetProcAddress(g_shv, "?nativeCall@@YAPEA_KXZ");

        if (!g_scriptWait || !g_scriptRegister || !g_scriptUnregister ||
            !g_nativeInit || !g_nativePush64 || !g_nativeCall)
            return FALSE;

        g_scriptRegister(hModule, ScriptMain);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        if (g_scriptUnregister && g_shv)
            g_scriptUnregister(g_self);
    }
    return TRUE;
}
