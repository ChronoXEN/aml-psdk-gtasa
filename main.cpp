#include <mod/amlmod.h>
#include <mod/config.h>
#include <jni.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

// --- 1. THE SIMPLE LOGGER ---
FILE* logFile = nullptr;

void OpenLog() {
    // This path is usually writable by the game on ALL Android versions
    logFile = fopen("/storage/emulated/0/Android/data/com.rockstargames.gtasa/files/leg_debug.txt", "w");
    if (logFile) {
        fprintf(logFile, "=== LEG IK DEBUG LOG STARTED ===\n");
        fflush(logFile);
    }
}

void LOG(const char* fmt, ...) {
    if (!logFile) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(logFile, fmt, args);
    fprintf(logFile, "\n");
    fflush(logFile); // Force write immediately so we don't lose data on crash
    va_end(args);
}

// --- 2. STRUCTURES ---
struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };

// Wrapper (AnimBlendFrameData)
struct AnimBlendFrameData {
    uint32_t flags;
    RwV3d    vecOffset;
    RwFrame* pFrame; 
};

// --- 3. SYMBOLS ---
void* (*RpAnimBlendClumpFindBone)(void* clump, unsigned int boneID) = nullptr;

IAMLer* aml = new IAMLer();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- 4. LOGIC ---
bool hasLogged = false; // Log only the first frame

void ApplyBoneBend(void* clump) {
    if (!RpAnimBlendClumpFindBone) return;

    // Try to find Left Calf (ID 3)
    void* wrapper = RpAnimBlendClumpFindBone(clump, 3);

    if (!hasLogged) LOG("  > RpAnimBlendClumpFindBone returned: %p", wrapper);

    if (!wrapper) return;

    // Extract RwFrame (Try offset 0x10)
    RwFrame* frame = *(RwFrame**)((uintptr_t)wrapper + 0x10);
    
    if (!hasLogged) LOG("  > RwFrame at offset 0x10: %p", frame);

    // Sanity Check
    if ((uintptr_t)frame < 0x10000) {
        if (!hasLogged) LOG("  > Pointer looks invalid, trying 0x14...");
        frame = *(RwFrame**)((uintptr_t)wrapper + 0x14);
        if (!hasLogged) LOG("  > RwFrame at offset 0x14: %p", frame);
    }

    if (!frame) return;

    // FORCE BEND: 90 Degrees
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

DECL_HOOKv(CPed_PreRender, void* self) {
    CPed_PreRender(self); 

    if (self) {
        if (!hasLogged) LOG("Hook: CPed_PreRender called for %p", self);

        // Clump Offset 0x18
        void* clump = *(void**)((uintptr_t)self + 0x18);

        if (!hasLogged) LOG("  > Clump at 0x18: %p", clump);

        if (clump) ApplyBoneBend(clump);
        
        hasLogged = true; // Stop logging
    }
}

extern "C" void OnModLoad() {
    OpenLog();
    LOG("Mod Loaded. Finding Symbols...");

    void* hGame = aml->GetLibHandle("libGTASA.so");
    if (!hGame) {
        LOG("CRITICAL: Failed to get libGTASA.so handle");
        return;
    }

    // Load Symbol
    RpAnimBlendClumpFindBone = (void*(*)(void*, unsigned int))aml->GetSym(hGame, "_Z24RpAnimBlendClumpFindBoneP7RpClumpj");
    if (RpAnimBlendClumpFindBone) LOG("Success: Found RpAnimBlendClumpFindBone");
    else LOG("ERROR: RpAnimBlendClumpFindBone NOT FOUND");

    // Hook
    void* hookAddr = aml->GetSym(hGame, "_ZN4CPed9PreRenderEv");
    if (hookAddr) {
        HOOK(CPed_PreRender, hookAddr);
        LOG("Success: Hooked CPed::PreRender");
    } else {
        LOG("ERROR: PreRender Symbol NOT FOUND");
    }
}
