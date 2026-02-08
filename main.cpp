#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>
#include <stdio.h> // For file logging

// --- 1. DEFINITIONS ---
struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };

// The Wrapper (AnimBlendFrameData)
struct AnimBlendFrameData {
    uint32_t flags;
    RwV3d    vecOffset;
    RwFrame* pFrame; // We hope this is at 0x10
};

// --- 2. SYMBOLS ---
void* (*RpAnimBlendClumpFindBone)(void* clump, unsigned int boneID) = nullptr;
void (*RwFrameUpdateObjects)(RwFrame* frame) = nullptr;

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- 3. FILE LOGGER ---
// This writes debug info to your SD card so we can see what's failing.
void LogToSD(const char* fmt, ...) {
    FILE* f = fopen("/sdcard/legik.log", "a");
    if (f) {
        va_list args;
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        fprintf(f, "\n");
        va_end(args);
        fclose(f);
    }
}

// --- 4. LOGIC ---
void ApplyBoneBend(void* clump) {
    // 1. Try to find the Left Calf (ID 3)
    void* wrapper = RpAnimBlendClumpFindBone(clump, 3);
    
    if(!wrapper) {
        // LogToSD("Error: Bone 3 not found via RpAnimBlendClumpFindBone");
        return;
    }

    // 2. Extract RwFrame (Try offset 0x10)
    RwFrame* frame = *(RwFrame**)((uintptr_t)wrapper + 0x10);
    
    // Safety: If pointer looks garbage, try 0x14
    if((uintptr_t)frame < 0x100000) {
         frame = *(RwFrame**)((uintptr_t)wrapper + 0x14);
    }

    if(!frame) {
        // LogToSD("Error: Found wrapper, but RwFrame is NULL");
        return;
    }

    // 3. FORCE TEST: ROTATE 90 DEGREES
    // We modify the matrix manually
    float angle = 90.0f;
    float rad = angle * (3.14159f / 180.0f);
    float c = cosf(rad);
    float s = sinf(rad);

    // Rotate Up and At around Right (X-Axis)
    RwV3d oldUp = frame->modelingMtx.up;
    RwV3d oldAt = frame->modelingMtx.at;

    frame->modelingMtx.up.y = oldUp.y * c - oldUp.z * s;
    frame->modelingMtx.up.z = oldUp.y * s + oldUp.z * c;
    frame->modelingMtx.at.y = oldAt.y * c - oldAt.z * s;
    frame->modelingMtx.at.z = oldAt.y * s + oldAt.z * c;

    // Set Dirty Flag (Force Update)
    frame->modelingMtx.flags |= 0x2;

    // 4. Update Objects (Critical for Visuals)
    if(RwFrameUpdateObjects) {
        RwFrameUpdateObjects(frame);
    }
}

// Global flag to log only once (spam prevention)
bool hasLogged = false;

DECL_HOOKv(CPed_PreRender, void* self) {
    CPed_PreRender(self); 
    
    if(self) {
        // Offset 0x18 is Clump
        void* clump = *(void**)((uintptr_t)self + 0x18);
        
        if (!hasLogged) {
            LogToSD("--- MOD STARTED ---");
            LogToSD("Hook: CPed_PreRender called!");
            LogToSD("Player Pointer: %p", self);
            LogToSD("Clump Pointer (at 0x18): %p", clump);
            if(clump) {
                void* testBone = RpAnimBlendClumpFindBone(clump, 3);
                LogToSD("Bone 3 Wrapper: %p", testBone);
                if(testBone) {
                     RwFrame* f = *(RwFrame**)((uintptr_t)testBone + 0x10);
                     LogToSD("RwFrame at 0x10: %p", f);
                }
            }
            hasLogged = true; // Stop logging after first frame
        }

        if(clump) ApplyBoneBend(clump);
    }
}

extern "C" void OnModLoad() {
    // Clear the log file on startup
    FILE* f = fopen("/sdcard/legik.log", "w");
    if(f) { fprintf(f, "Mod Loaded.\n"); fclose(f); }

    void* hGame = aml->GetLibHandle("libGTASA.so");
    
    // Load Functions
    RpAnimBlendClumpFindBone = (void*(*)(void*, unsigned int))aml->GetSym(hGame, "_Z24RpAnimBlendClumpFindBoneP7RpClumpj");
    RwFrameUpdateObjects = (void(*)(RwFrame*))aml->GetSym(hGame, "_Z20RwFrameUpdateObjectsP7RwFrame");
    
    // Install Hook
    HOOK(CPed_PreRender, aml->GetSym(hGame, "_ZN4CPed9PreRenderEv"));
}
