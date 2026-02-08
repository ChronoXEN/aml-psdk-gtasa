#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>

// --- 1. DEFINITIONS ---
struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };

// --- 2. THE MAGICAL FUNCTION YOU FOUND ---
// _Z24RpAnimBlendClumpFindBoneP7RpClumpj -> RpAnimBlendClumpFindBone(RpClump*, uint)
void* (*RpAnimBlendClumpFindBone)(void* clump, unsigned int boneID) = nullptr;

// Helper to rotate
void (*RwFrameRotate)(RwFrame* frame, RwV3d* axis, float angle, int combine) = nullptr;
void (*RwFrameUpdateObjects)(RwFrame* frame) = nullptr;

// Constants
#define rwCOMBINEREPLACE 0 
#define rwCOMBINEPRECONCAT 1

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)


// --- 3. THE LOGIC ---

void ApplyBoneBend(void* clump) {
    if(!clump || !RpAnimBlendClumpFindBone || !RwFrameRotate) return;

    // USE YOUR FUNCTION TO FIND BONE 3 (Left Calf)
    // 3 = BONE_L_CALF in GTA SA
    void* bonePtr = RpAnimBlendClumpFindBone(clump, 3);
    
    if(!bonePtr) return;

    // Cast it to RwFrame (Standard RenderWare structure)
    RwFrame* calfFrame = (RwFrame*)bonePtr;

    // FORCE TEST: 90 Degrees Backward
    RwV3d axis = { 1.0f, 0.0f, 0.0f }; // X-Axis
    float angle = 90.0f;

    // Apply Rotation
    RwFrameRotate(calfFrame, &axis, angle, rwCOMBINEREPLACE);
    
    // Update Physics/Children
    if(RwFrameUpdateObjects) RwFrameUpdateObjects(calfFrame);
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
    
    // 4. LOAD YOUR SYMBOL
    RpAnimBlendClumpFindBone = (void*(*)(void*, unsigned int))aml->GetSym(hGame, "_Z24RpAnimBlendClumpFindBoneP7RpClumpj");
    
    // Load Helpers
    RwFrameRotate = (void(*)(RwFrame*, RwV3d*, float, int))aml->GetSym(hGame, "_Z13RwFrameRotateP7RwFramePK5RwV3d19RwOpCombineType");
    RwFrameUpdateObjects = (void(*)(RwFrame*))aml->GetSym(hGame, "_Z20RwFrameUpdateObjectsP7RwFrame");

    // Hook
    HOOK(CPed_PreRender, aml->GetSym(hGame, "_ZN4CPed9PreRenderEv"));
}
