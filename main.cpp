#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>

// --- DEFINITIONS ---
struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };
struct RpHAnimNodeInfo { int32_t nodeID; int32_t index; int32_t flags; RwFrame* pFrame; };
struct RpHAnimHierarchy { int32_t flags; int32_t numNodes; RwMatrix* pMatrixArray; void* pMatrixArrayUnaligned; RpHAnimNodeInfo* pNodeInfo; };

// --- FUNCTION POINTERS ---
void* (*GetAnimHierarchyFromSkinClump)(void* clump) = nullptr;
void (*RwFrameRotate)(RwFrame* frame, RwV3d* axis, float angle, int combine) = nullptr;
void (*RwFrameUpdateObjects)(RwFrame* frame) = nullptr;

// Constants
#define rwCOMBINEPRECONCAT 1

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- FORCE LEAN LOGIC ---
void ApplyForceLean(void* pRwClump) {
    if (!pRwClump || !GetAnimHierarchyFromSkinClump || !RwFrameRotate) return;

    RpHAnimHierarchy* hier = (RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(pRwClump);
    if (!hier) return;

    // Find ROOT (Bone 0) - This is the Pelvis.
    // If we rotate this, the WHOLE character should tilt.
    int rootIdx = -1;
    for(int i=0; i<hier->numNodes; i++) {
        if(hier->pNodeInfo[i].nodeID == 0) { // 0 = Root/Pelvis
            rootIdx = i;
            break;
        }
    }

    if (rootIdx == -1) return;

    RwFrame* rootFrame = hier->pNodeInfo[rootIdx].pFrame;

    // FORCE LEAN: 45 Degrees sideways
    RwV3d axis = { 0.0f, 1.0f, 0.0f }; // Y-Axis (Sideways)
    float angle = 45.0f; 

    // Apply Rotation
    RwFrameRotate(rootFrame, &axis, angle, rwCOMBINEPRECONCAT);
    
    // Update hierarchy so the legs/arms move with the pelvis
    if(RwFrameUpdateObjects) RwFrameUpdateObjects(rootFrame);
}

// --- THE HOOK: CPed::PreRender ---
// This is the "Sweet Spot" for IK.
DECL_HOOKv(CPed_PreRender, void* self) {
    // 1. Run original logic first (calculates animations)
    CPed_PreRender(self); 
    
    // 2. NOW apply our bend before drawing
    if(self) {
        // Try offset 0x18 (Standard for v2.00/v2.10)
        void* clump = *(void**)((uintptr_t)self + 0x18);
        if(clump) ApplyForceLean(clump);
    }
}

// --- MOD LOAD ---
extern "C" void OnModLoad() {
    logger->SetTag("LegIK");
    void* hGame = aml->GetLibHandle("libGTASA.so");
    
    // Load Functions
    GetAnimHierarchyFromSkinClump = (void*(*)(void*))aml->GetSym(hGame, "_Z29GetAnimHierarchyFromSkinClumpP7RpClump");
    RwFrameRotate = (void(*)(RwFrame*, RwV3d*, float, int))aml->GetSym(hGame, "_Z13RwFrameRotateP7RwFramePK5RwV3d19RwOpCombineType");
    RwFrameUpdateObjects = (void(*)(RwFrame*))aml->GetSym(hGame, "_Z20RwFrameUpdateObjectsP7RwFrame");

    // Hook PreRender
    HOOK(CPed_PreRender, aml->GetSym(hGame, "_ZN4CPed9PreRenderEv"));
}
