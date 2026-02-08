#include <mod/amlmod.h>
#include <mod/logger.h>
#include <mod/config.h>
#include <jni.h>
#include <math.h>

// -------------------------------------------------------------------------
// 1. MANUAL DEFINITIONS (The "Dictionary" for the Compiler)
// -------------------------------------------------------------------------

struct RwV3d { float x, y, z; };
struct RwMatrix { RwV3d right; uint32_t flags; RwV3d up; uint32_t pad1; RwV3d at; uint32_t pad2; RwV3d pos; uint32_t pad3; };
struct RwFrame { RwMatrix modelingMtx; void* parent; void* next; void* root; };
struct RpHAnimNodeInfo { int32_t nodeID; int32_t index; int32_t flags; RwFrame* pFrame; };
struct RpHAnimHierarchy { int32_t flags; int32_t numNodes; RwMatrix* pMatrixArray; void* pMatrixArrayUnaligned; RpHAnimNodeInfo* pNodeInfo; };

// Function Pointers (We will find these addresses when the mod loads)
void* (*GetAnimHierarchyFromSkinClump)(void* clump) = nullptr;
void (*RwFrameRotate)(RwFrame* frame, RwV3d* axis, float angle, int combine) = nullptr;
void (*RwFrameUpdateObjects)(RwFrame* frame) = nullptr;
void* (*FindPlayerPed)(int id) = nullptr;

// Constants
#define rwCOMBINEPRECONCAT 1

// Fake SDK Objects
IAMLer* aml = new IAMLer();
FakeLogger* logger = new FakeLogger();

MYMODCFG(net.rusjj.legik, LegIK Mod, 1.0, YourName)

// -------------------------------------------------------------------------
// 2. THE FORCE TEST LOGIC
// -------------------------------------------------------------------------

void ApplyLegIK(void* pRwClump) {
    // Safety Checks: If we haven't found the game functions yet, don't crash.
    if (!pRwClump || !GetAnimHierarchyFromSkinClump || !RwFrameRotate) return;

    RpHAnimHierarchy* hier = (RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(pRwClump);
    if (!hier) return;

    // Find Bone Indices Manually (2 = L_Thigh, 3 = L_Calf)
    int thighIdx = -1, calfIdx = -1;
    for(int i=0; i<hier->numNodes; i++) {
        if(hier->pNodeInfo[i].nodeID == 2) thighIdx = i; 
        if(hier->pNodeInfo[i].nodeID == 3) calfIdx = i;  
    }

    if (thighIdx == -1 || calfIdx == -1) return;

    RwFrame* calfFrame = hier->pNodeInfo[calfIdx].pFrame;
    RwFrame* thighFrame = hier->pNodeInfo[thighIdx].pFrame;

    // --- FORCE TEST ---
    // We force a 45 degree bend on the X axis.
    RwV3d axis = { 1.0f, 0.0f, 0.0f }; 
    float angle = 45.0f; 

    // Apply the rotation
    RwFrameRotate(calfFrame, &axis, angle, rwCOMBINEPRECONCAT);
    
    // Update the parent (Thigh) so the physics engine knows we moved the calf
    if(RwFrameUpdateObjects) RwFrameUpdateObjects(thighFrame);
}

// -------------------------------------------------------------------------
// 3. THE HOOK (Runs Every Frame)
// -------------------------------------------------------------------------

DECL_HOOKv(CTimer_Update) {
    CTimer_Update(); // Let the game do its normal update first

    if(FindPlayerPed) {
        void* player = FindPlayerPed(-1);
        if(player) {
             // Access m_pRwClump at offset 0x18 (Standard for Android)
             void* clump = *(void**)((uintptr_t)player + 0x18);
             ApplyLegIK(clump);
        }
    }
}

// -------------------------------------------------------------------------
// 4. MOD LOAD (Finds the Functions)
// -------------------------------------------------------------------------

extern "C" void OnModLoad() {
    logger->SetTag("LegIK");
    
    // Get the Game Library
    void* hGame = aml->GetLibHandle("libGTASA.so");
    
    // Look up the symbols (Variable Names) in the game memory
    GetAnimHierarchyFromSkinClump = (void*(*)(void*))aml->GetSym(hGame, "_Z29GetAnimHierarchyFromSkinClumpP7RpClump");
    RwFrameRotate = (void(*)(RwFrame*, RwV3d*, float, int))aml->GetSym(hGame, "_Z13RwFrameRotateP7RwFramePK5RwV3d19RwOpCombineType");
    RwFrameUpdateObjects = (void(*)(RwFrame*))aml->GetSym(hGame, "_Z20RwFrameUpdateObjectsP7RwFrame");
    FindPlayerPed = (void*(*)(int))aml->GetSym(hGame, "_Z13FindPlayerPedi");

    // Install the Hook
    HOOK(CTimer_Update, aml->GetSym(hGame, "_ZN6CTimer6UpdateEv"));
}
