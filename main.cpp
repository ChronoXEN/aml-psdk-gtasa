#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>
#include <stdio.h>
#include <android/log.h> // Official Android Logging

// --- 1. DEFINITIONS ---
struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };

// --- 2. LOGGING HELPERS ---
// Writes to System Log (Viewable with Logcat apps)
#define LOG_TAG "LegIK_Debug"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Writes to File in the Game Folder (Allowed by Android 11+)
void LogToFile(const char* fmt, ...) {
    // This path is usually writable by the game
    FILE* f = fopen("/storage/emulated/0/Android/data/com.rockstargames.gtasa/files/legik.log", "a");
    if (f) {
        va_list args;
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        fprintf(f, "\n");
        va_end(args);
        fclose(f);
    } else {
        // Fallback: If that fails, try the older path just in case
        FILE* f2 = fopen("/sdcard/Android/data/com.rockstargames.gtasa/files/legik.log", "a");
        if (f2) {
             va_list args;
             va_start(args, fmt);
             vfprintf(f2, fmt, args);
             fprintf(f2, "\n");
             va_end(args);
             fclose(f2);
        }
    }
}

// --- 3. SYMBOLS ---
// Wrapper: RpAnimBlendClumpFindBone(RpClump*, uint)
void* (*RpAnimBlendClumpFindBone)(void* clump, unsigned int boneID) = nullptr;

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)


// --- 4. LOGIC ---
bool hasLogged = false;

DECL_HOOKv(CPed_PreRender, void* self) {
    CPed_PreRender(self); 
    
    // Only log the first frame to avoid freezing the phone
    if (!hasLogged) {
        LOGI("Hook CPed_PreRender hit! Player Pointer: %p", self);
        LogToFile("Hook CPed_PreRender hit! Player Pointer: %p", self);
    }

    if(self) {
        // 1. GET CLUMP (Offset 0x18)
        void* clump = *(void**)((uintptr_t)self + 0x18);
        
        if (!hasLogged) {
            LOGI("Clump Pointer at 0x18: %p", clump);
            LogToFile("Clump Pointer at 0x18: %p", clump);
        }

        if(clump) {
            // 2. FIND BONE 3 (Left Calf)
            if (RpAnimBlendClumpFindBone) {
                void* wrapper = RpAnimBlendClumpFindBone(clump, 3);
                
                if (!hasLogged) {
                    LOGI("Bone 3 Wrapper: %p", wrapper);
                    LogToFile("Bone 3 Wrapper: %p", wrapper);
                }

                if(wrapper) {
                    // 3. EXTRACT RWFRAME (Offset 0x10 or 0x14)
                    // We assume 0x10 for now
                    RwFrame* frame = *(RwFrame**)((uintptr_t)wrapper + 0x10);
                    
                    if (!hasLogged) {
                        LOGI("RwFrame at 0x10: %p", frame);
                        LogToFile("RwFrame at 0x10: %p", frame);
                    }

                    if(frame) {
                        // 4. APPLY ROTATION (Matrix Math)
                        // Rotate 90 degrees on X-axis
                        float angle = 90.0f;
                        float rad = angle * (3.14159f / 180.0f);
                        float c = cosf(rad);
                        float s = sinf(rad);

                        RwV3d oldUp = frame->modelingMtx.up;
                        RwV3d oldAt = frame->modelingMtx.at;

                        frame->modelingMtx.up.y = oldUp.y * c - oldUp.z * s;
                        frame->modelingMtx.up.z = oldUp.y * s + oldUp.z * c;
                        frame->modelingMtx.at.y = oldAt.y * c - oldAt.z * s;
                        frame->modelingMtx.at.z = oldAt.y * s + oldAt.z * c;

                        // Dirty Flag
                        frame->modelingMtx.flags |= 0x2;
                    }
                }
            } else {
                if (!hasLogged) LogToFile("Error: RpAnimBlendClumpFindBone symbol is missing!");
            }
        }
        hasLogged = true;
    }
}

extern "C" void OnModLoad() {
    // Init Logs
    LogToFile("--- MOD LOADING ---");
    LOGI("--- MOD LOADING ---");

    void* hGame = aml->GetLibHandle("libGTASA.so");
    if (!hGame) {
        LogToFile("Error: Could not find libGTASA.so handle!");
        return;
    }
    
    // Load Function
    RpAnimBlendClumpFindBone = (void*(*)(void*, unsigned int))aml->GetSym(hGame, "_Z24RpAnimBlendClumpFindBoneP7RpClumpj");
    
    if (!RpAnimBlendClumpFindBone) {
        LogToFile("Error: Symbol _Z24RpAnimBlendClumpFindBoneP7RpClumpj NOT FOUND.");
    } else {
        LogToFile("Success: Found RpAnimBlendClumpFindBone.");
    }
    
    // Hook
    void* hookAddr = aml->GetSym(hGame, "_ZN4CPed9PreRenderEv");
    if (hookAddr) {
        HOOK(CPed_PreRender, hookAddr);
        LogToFile("Success: Hooked CPed::PreRender.");
    } else {
        LogToFile("Error: Symbol _ZN4CPed9PreRenderEv NOT FOUND.");
    }
}
