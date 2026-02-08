#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>

// --- STRUCTURES ---
struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };

// The Wrapper you get from RpAnimBlendClumpFindBone
struct AnimBlendFrameData {
    uint32_t flags;
    RwV3d    vecOffset;
    RwFrame* pFrame; // <--- THIS is what we want. Usually at offset 0x10 or 0x14.
};

// --- SYMBOLS ---
// RpAnimBlendClumpFindBone(RpClump*, uint)
void* (*RpAnimBlendClumpFindBone)(void* clump, unsigned int boneID) = nullptr;

// We don't need RwFrameRotate anymore. We will do the math ourselves.

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- MANUAL ROTATION MATH ---
// Rotates a matrix on the X-axis (Pitch) by 'angle' degrees
void RotateMatrixX(RwMatrix* mtx, float angle) {
    float rad = angle * (3.14159f / 180.0f);
    float c = cosf(rad);
    float s = sinf(rad);

    // GTA Matrices: Up = Y, At = Z.
    // Rotate Up and At around Right (X)
    RwV3d oldUp = mtx->up;
    RwV3d oldAt = mtx->at;

    mtx->up.y = oldUp.y * c - oldUp.z * s;
    mtx->up.z = oldUp.y * s + oldUp.z * c;

    mtx->at.y = oldAt.y * c - oldAt.z * s;
    mtx->at.z = oldAt.y * s + oldAt.z * c;
    
    // Dirty flag (0x2) tells the game "Recalculate this bone"
    // We just OR it in to be safe.
    mtx->flags |= 0x2; 
}

// --- LOGIC ---

void ApplyBoneBend(void* clump) {
    if(!clump || !RpAnimBlendClumpFindBone) return;

    // 1. Get the Wrapper (AnimBlendFrameData)
    // Bone 3 = Left Calf
    void* wrapper = RpAnimBlendClumpFindBone(clump, 3);
    
    if(!wrapper) return;

    // 2. Extract the Real Bone (RwFrame)
    // In GTA SA Android, the RwFrame* is usually at offset 0x10 or 0x14 of the wrapper.
    // Let's try 0x10 first (Standard for 32-bit).
    // wrapper + 16 bytes = pFrame?
    
    RwFrame* frame = *(RwFrame**)((uintptr_t)wrapper + 0x10);
    
    // Safety check: Does it look like a valid pointer?
    if((uintptr_t)frame < 0x10000) {
        // Try offset 0x14 just in case
        frame = *(RwFrame**)((uintptr_t)wrapper + 0x14);
    }

    if(!frame) return;

    // 3. FORCE TEST: 90 Degrees
    RotateMatrixX(&frame->modelingMtx, 90.0f);
    
    // Note: We don't call RwFrameUpdateObjects. 
    // Since we hooked PreRender, the game will update the children automatically when it draws.
}

// Hook CPed::PreRender
DECL_HOOKv(CPed_PreRender, void* self) {
    CPed_PreRender(self); 
    
    if(self) {
        // Get Clump from Offset 0x18
        void* clump = *(void**)((uintptr_t)self + 0x18);
        if(clump) ApplyBoneBend(clump);
    }
}

extern "C" void OnModLoad() {
    logger->SetTag("LegIK");
    void* hGame = aml->GetLibHandle("libGTASA.so");
    
    // Load YOUR symbol
    RpAnimBlendClumpFindBone = (void*(*)(void*, unsigned int))aml->GetSym(hGame, "_Z24RpAnimBlendClumpFindBoneP7RpClumpj");
    
    // Hook
    HOOK(CPed_PreRender, aml->GetSym(hGame, "_ZN4CPed9PreRenderEv"));
}
