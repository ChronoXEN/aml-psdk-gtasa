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
#define rwCOMBINEREPLACE 0  // Replaces the animation entirely (Strongest)
#define rwCOMBINEPRECONCAT 1 // Adds to the animation (Smoother)

IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();
MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// --- LEG BENDING LOGIC ---
void ApplyForceBend(void* pRwClump) {
    if (!pRwClump || !GetAnimHierarchyFromSkinClump || !RwFrameRotate) return;

    RpHAnimHierarchy* hier = (RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(pRwClump);
    if (!hier) return;

    // Find Left Calf (ID 3)
    int calfIdx = -1;
    for(int i=0; i<hier->numNodes; i++) {
        if(hier->pNodeInfo[i].nodeID == 3) {
            calfIdx = i;
            break;
        }
    }

    if (calfIdx == -1) return;

    RwFrame* calfFrame = hier->pNodeInfo[calfIdx].pFrame;

    // FORCE TEST: Bend leg 90 degrees BACKWARDS
    RwV3d axis = { 1.0f, 0.0f, 0.0f }; 
    float angle = 90.0f; 

    // We use REPLACE to make sure the game doesn't overwrite us
    RwFrameRotate(calfFrame, &axis, angle, rwCOMBINEREPLACE);
    
    // Note: We don't update parent here because we want to see if the rotation sticks at all
    if(RwFrameUpdateObjects) RwFrameUpdateObjects(calfFrame);
}

// --- THE NEW HOOK (Runs during Rendering) ---
// This function runs exactly when the game draws the Ped.
// 'self' is the CPed pointer.

DECL_HOOKv(CPed_Render, void* self) {
    // 1. Run the original game rendering first
    CPed_Render(self); 
    
    // 2. NOW we bend the leg. 
    // Since the game just finished calculating animations, we are modifying the result.
    // We need to fetch the Clump from the CPed struct.
    // Offset 0x18 is standard for 'm_pRwClump' in 32-bit GTA SA.
    if(self) {
        void* clump = *(void**)((uintptr_t)self + 0x18);
        ApplyForceBend(clump);
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

    // Hook CPed::Render instead of CTimer::Update
    HOOK(CPed_Render, aml->GetSym(hGame, "_ZN4CPed6RenderEv"));
}
