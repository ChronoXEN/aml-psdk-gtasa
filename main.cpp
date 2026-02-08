#include <mod/amlmod.h>
#include <mod/logger.h> // Now actually works!
#include <mod/config.h>
#include <jni.h>
#include <math.h>

// --- STRUCTURES ---
struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };

// --- SYMBOLS ---
// RpAnimBlendClumpFindBone(RpClump*, uint)
void* (*RpAnimBlendClumpFindBone)(void* clump, unsigned int boneID) = nullptr;

IAMLer* aml = new IAMLer();
// logger is defined in logger.cpp now
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- LOGIC ---
bool hasLogged = false;

DECL_HOOKv(CPed_PreRender, void* self) {
    CPed_PreRender(self); 
    
    // Log ONCE only
    if (!hasLogged) logger->Info("Hook CPed_PreRender HIT! Player: %p", self);

    if(self) {
        // 1. GET CLUMP (Offset 0x18)
        void* clump = *(void**)((uintptr_t)self + 0x18);
        
        if (!hasLogged) logger->Info("Clump Pointer (0x18): %p", clump);

        if(clump) {
            // 2. FIND BONE 3 (Left Calf)
            if (RpAnimBlendClumpFindBone) {
                void* wrapper = RpAnimBlendClumpFindBone(clump, 3);
                
                if (!hasLogged) logger->Info("Bone 3 Wrapper: %p", wrapper);

                if(wrapper) {
                    // 3. EXTRACT RWFRAME (Try 0x10)
                    RwFrame* frame = *(RwFrame**)((uintptr_t)wrapper + 0x10);
                    
                    if (!hasLogged) logger->Info("RwFrame at 0x10: %p", frame);

                    // Safety Check: Is pointer valid?
                    if((uintptr_t)frame > 0x100000) {
                        
                        // 4. APPLY ROTATION (90 Degrees X-Axis)
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
                    } else {
                        if (!hasLogged) logger->Info("ERROR: Frame pointer looks invalid.");
                    }
                }
            } else {
                if (!hasLogged) logger->Info("ERROR: Symbol RpAnimBlendClumpFindBone is NULL.");
            }
        }
        hasLogged = true;
    }
}

extern "C" void OnModLoad() {
    logger->Info("--- MOD LOADED ---");

    void* hGame = aml->GetLibHandle("libGTASA.so");
    if (!hGame) {
        logger->Info("FATAL: Could not get libGTASA.so handle");
        return;
    }
    
    // Load Function
    RpAnimBlendClumpFindBone = (void*(*)(void*, unsigned int))aml->GetSym(hGame, "_Z24RpAnimBlendClumpFindBoneP7RpClumpj");
    if (RpAnimBlendClumpFindBone) logger->Info("Found RpAnimBlendClumpFindBone");
    else logger->Info("ERROR: Could not find RpAnimBlendClumpFindBone");
    
    // Hook
    void* hookAddr = aml->GetSym(hGame, "_ZN4CPed9PreRenderEv");
    if (hookAddr) {
        HOOK(CPed_PreRender, hookAddr);
        logger->Info("Hooked CPed::PreRender");
    } else {
        logger->Info("ERROR: Could not find CPed::PreRender symbol");
    }
}
